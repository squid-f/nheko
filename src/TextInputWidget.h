/*
 * nheko Copyright (C) 2017  Konstantinos Sideris <siderisk@auth.gr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <deque>
#include <iterator>
#include <map>

#include <QApplication>
#include <QDebug>
#include <QHBoxLayout>
#include <QPaintEvent>
#include <QTextEdit>
#include <QWidget>

#include "SuggestionsPopup.h"
#include "dialogs/PreviewUploadOverlay.h"
#include "emoji/PickButton.h"

namespace dialogs {
class PreviewUploadOverlay;
}

struct SearchResult;

class FlatButton;
class LoadingIndicator;

class FilteredTextEdit : public QTextEdit
{
        Q_OBJECT

public:
        explicit FilteredTextEdit(QWidget *parent = nullptr);

        void stopTyping();

        QSize sizeHint() const override;
        QSize minimumSizeHint() const override;

        void submit();
        void setRelatedEvent(const QString &event) { related_event_ = event; }

signals:
        void heightChanged(int height);
        void startedTyping();
        void stoppedTyping();
        void startedUpload();
        void message(QString);
        void reply(QString, QString);
        void command(QString name, QString args);
        void image(QSharedPointer<QIODevice> data, const QString &filename);
        void audio(QSharedPointer<QIODevice> data, const QString &filename);
        void video(QSharedPointer<QIODevice> data, const QString &filename);
        void file(QSharedPointer<QIODevice> data, const QString &filename);

        //! Trigger the suggestion popup.
        void showSuggestions(const QString &query);
        void resultsRetrieved(const QVector<SearchResult> &results);
        void selectNextSuggestion();
        void selectPreviousSuggestion();
        void selectHoveredSuggestion();

public slots:
        void showResults(const QVector<SearchResult> &results);

protected:
        void keyPressEvent(QKeyEvent *event) override;
        bool canInsertFromMimeData(const QMimeData *source) const override;
        void insertFromMimeData(const QMimeData *source) override;
        void focusOutEvent(QFocusEvent *event) override
        {
                popup_.hide();
                QTextEdit::focusOutEvent(event);
        }

private:
        std::deque<QString> true_history_, working_history_;
        size_t history_index_;
        QTimer *typingTimer_;

        SuggestionsPopup popup_;

        // Used for replies
        QString related_event_;

        enum class AnchorType
        {
                Tab   = 0,
                Sigil = 1,
        };

        AnchorType anchorType_ = AnchorType::Sigil;

        int anchorWidth(AnchorType anchor) { return static_cast<int>(anchor); }

        void closeSuggestions() { popup_.hide(); }
        void resetAnchor() { atTriggerPosition_ = -1; }
        bool isAnchorValid() { return atTriggerPosition_ != -1; }
        bool hasAnchor(int pos, AnchorType anchor)
        {
                return pos == atTriggerPosition_ + anchorWidth(anchor);
        }

        QString query()
        {
                auto cursor = textCursor();
                cursor.movePosition(QTextCursor::StartOfWord, QTextCursor::KeepAnchor);
                return cursor.selectedText();
        }

        dialogs::PreviewUploadOverlay previewDialog_;

        //! Latest position of the '@' character that triggers the username completer.
        int atTriggerPosition_ = -1;

        void textChanged();
        void uploadData(const QByteArray data, const QString &media, const QString &filename);
        void afterCompletion(int);
        void showPreview(const QMimeData *source, const QStringList &formats);
};

class TextInputWidget : public QWidget
{
        Q_OBJECT

        Q_PROPERTY(QColor borderColor READ borderColor WRITE setBorderColor)

public:
        TextInputWidget(QWidget *parent = 0);

        void stopTyping();

        QColor borderColor() const { return borderColor_; }
        void setBorderColor(QColor &color) { borderColor_ = color; }
        void disableInput()
        {
                input_->setEnabled(false);
                input_->setPlaceholderText(tr("Connection lost. Nheko is trying to re-connect..."));
        }
        void enableInput()
        {
                input_->setEnabled(true);
                input_->setPlaceholderText(tr("Write a message..."));
        }

public slots:
        void openFileSelection();
        void hideUploadSpinner();
        void focusLineEdit() { input_->setFocus(); }
        void addReply(const QString &username, const QString &msg, const QString &related_event);

private slots:
        void addSelectedEmoji(const QString &emoji);

signals:
        void sendTextMessage(QString msg);
        void sendReplyMessage(QString msg, QString event_id);
        void sendEmoteMessage(QString msg);
        void heightChanged(int height);

        void uploadImage(const QSharedPointer<QIODevice> data, const QString &filename);
        void uploadFile(const QSharedPointer<QIODevice> data, const QString &filename);
        void uploadAudio(const QSharedPointer<QIODevice> data, const QString &filename);
        void uploadVideo(const QSharedPointer<QIODevice> data, const QString &filename);

        void sendJoinRoomRequest(const QString &room);

        void startedTyping();
        void stoppedTyping();

protected:
        void focusInEvent(QFocusEvent *event) override;
        void paintEvent(QPaintEvent *) override;

private:
        void showUploadSpinner();
        void command(QString name, QString args);

        QHBoxLayout *topLayout_;
        FilteredTextEdit *input_;

        // Used for replies
        QString related_event_;

        LoadingIndicator *spinner_;

        FlatButton *sendFileBtn_;
        FlatButton *sendMessageBtn_;
        emoji::PickButton *emojiBtn_;

        QColor borderColor_;
};
