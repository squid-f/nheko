// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "MxcAnimatedImage.h"

#include <QDir>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QQuickWindow>
#include <QSGImageNode>
#include <QStandardPaths>

#include "EventAccessors.h"
#include "Logging.h"
#include "MatrixClient.h"
#include "timeline/TimelineModel.h"

void
MxcAnimatedImage::startDownload()
{
    if (!room_)
        return;
    if (eventId_.isEmpty())
        return;

    auto event = room_->eventById(eventId_);
    if (!event) {
        nhlog::ui()->error("Failed to load media for event {}, event not found.",
                           eventId_.toStdString());
        return;
    }

    QByteArray mimeType = QString::fromStdString(mtx::accessors::mimetype(*event)).toUtf8();

    animatable_ = QMovie::supportedFormats().contains(mimeType.split('/').back());
    animatableChanged();

    if (!animatable_)
        return;

    QString mxcUrl           = QString::fromStdString(mtx::accessors::url(*event));
    QString originalFilename = QString::fromStdString(mtx::accessors::filename(*event));

    auto encryptionInfo = mtx::accessors::file(*event);

    // If the message is a link to a non mxcUrl, don't download it
    if (!mxcUrl.startsWith("mxc://")) {
        return;
    }

    QString suffix = QMimeDatabase().mimeTypeForName(mimeType).preferredSuffix();

    const auto url  = mxcUrl.toStdString();
    const auto name = QString(mxcUrl).remove("mxc://");
    QFileInfo filename(QString("%1/media_cache/media/%2.%3")
                         .arg(QStandardPaths::writableLocation(QStandardPaths::CacheLocation))
                         .arg(name)
                         .arg(suffix));
    if (QDir::cleanPath(name) != name) {
        nhlog::net()->warn("mxcUrl '{}' is not safe, not downloading file", url);
        return;
    }

    QDir().mkpath(filename.path());

    QPointer<MxcAnimatedImage> self = this;

    auto processBuffer = [this, mimeType, encryptionInfo, self](QIODevice &device) {
        if (!self)
            return;

        if (buffer.isOpen()) {
            movie.stop();
            movie.setDevice(nullptr);
            buffer.close();
        }

        if (encryptionInfo) {
            QByteArray ba = device.readAll();
            std::string temp(ba.constData(), ba.size());
            temp = mtx::crypto::to_string(mtx::crypto::decrypt_file(temp, encryptionInfo.value()));
            buffer.setData(temp.data(), temp.size());
        } else {
            buffer.setData(device.readAll());
        }
        buffer.open(QIODevice::ReadOnly);
        buffer.reset();

        QTimer::singleShot(0, this, [this, mimeType] {
            nhlog::ui()->info(
              "Playing movie with size: {}, {}", buffer.bytesAvailable(), buffer.isOpen());
            movie.setFormat(mimeType);
            movie.setDevice(&buffer);
            if (play_)
                movie.start();
            else
                movie.jumpToFrame(0);
            emit loadedChanged();
            update();
        });
    };

    if (filename.isReadable()) {
        QFile f(filename.filePath());
        if (f.open(QIODevice::ReadOnly)) {
            processBuffer(f);
            return;
        }
    }

    http::client()->download(url,
                             [filename, url, processBuffer](const std::string &data,
                                                            const std::string &,
                                                            const std::string &,
                                                            mtx::http::RequestErr err) {
                                 if (err) {
                                     nhlog::net()->warn("failed to retrieve media {}: {} {}",
                                                        url,
                                                        err->matrix_error.error,
                                                        static_cast<int>(err->status_code));
                                     return;
                                 }

                                 try {
                                     QFile file(filename.filePath());

                                     if (!file.open(QIODevice::WriteOnly))
                                         return;

                                     QByteArray ba(data.data(), (int)data.size());
                                     file.write(ba);
                                     file.close();

                                     QBuffer buf(&ba);
                                     buf.open(QBuffer::ReadOnly);
                                     processBuffer(buf);
                                 } catch (const std::exception &e) {
                                     nhlog::ui()->warn("Error while saving file to: {}", e.what());
                                 }
                             });
}

QSGNode *
MxcAnimatedImage::updatePaintNode(QSGNode *oldNode, QQuickItem::UpdatePaintNodeData *)
{
    if (!imageDirty)
        return oldNode;

    imageDirty      = false;
    QSGImageNode *n = static_cast<QSGImageNode *>(oldNode);
    if (!n) {
        n = window()->createImageNode();
        n->setOwnsTexture(true);
        // n->setFlags(QSGNode::OwnedByParent | QSGNode::OwnsGeometry |
        // GSGNode::OwnsMaterial);
        n->setFlags(QSGNode::OwnedByParent);
    }

    // n->setTexture(nullptr);
    auto img = movie.currentImage();
    if (!img.isNull())
        n->setTexture(window()->createTextureFromImage(img));
    else {
        delete n;
        return nullptr;
    }

    n->setSourceRect(img.rect());
    n->setRect(QRect(0, 0, width(), height()));
    n->setFiltering(QSGTexture::Linear);
    n->setMipmapFiltering(QSGTexture::Linear);

    return n;
}
