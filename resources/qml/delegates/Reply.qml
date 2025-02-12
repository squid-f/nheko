// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

import Qt.labs.platform 1.1 as Platform
import QtQuick 2.12
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.2
import QtQuick.Window 2.13
import im.nheko 1.0

Item {
    id: r

    property color userColor: "red"
    property double proportionalHeight
    property int type
    property string typeString
    property int originalWidth
    property string blurhash
    property string body
    property string formattedBody
    property string eventId
    property string filename
    property string filesize
    property string url
    property bool isOnlyEmoji
    property string userId
    property string userName
    property string thumbnailUrl
    property string roomTopic
    property string roomName
    property string callType
    property int encryptionError
    property int relatedEventCacheBuster

    width: parent.width
    height: replyContainer.height

    CursorShape {
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
    }

    Rectangle {
        id: colorLine

        anchors.top: replyContainer.top
        anchors.bottom: replyContainer.bottom
        width: 4
        color: TimelineManager.userColor(userId, Nheko.colors.base)
    }

    Column {
        id: replyContainer

        anchors.left: colorLine.right
        anchors.leftMargin: 4
        width: parent.width - 8

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onSingleTapped: {
                let link = reply.child.linkAt(eventPoint.position.x, eventPoint.position.y - userName_.implicitHeight);
                if (link) {
                    Nheko.openLink(link)
                } else {
                    room.showEvent(r.eventId)
                }
            }
            gesturePolicy: TapHandler.ReleaseWithinBounds
        }

        TapHandler {
            acceptedButtons: Qt.RightButton
            onLongPressed: replyContextMenu.show(reply.child.copyText, reply.child.linkAt(eventPoint.position.x, eventPoint.position.y - userName_.implicitHeight))
            onSingleTapped: replyContextMenu.show(reply.child.copyText, reply.child.linkAt(eventPoint.position.x, eventPoint.position.y - userName_.implicitHeight))
            gesturePolicy: TapHandler.ReleaseWithinBounds
        }

        Text {
            id: userName_

            text: TimelineManager.escapeEmoji(userName)
            color: r.userColor
            textFormat: Text.RichText

            TapHandler {
                onSingleTapped: room.openUserProfile(userId)
                gesturePolicy: TapHandler.ReleaseWithinBounds
            }

        }

        MessageDelegate {
            id: reply

            blurhash: r.blurhash
            body: r.body
            formattedBody: r.formattedBody
            eventId: r.eventId
            filename: r.filename
            filesize: r.filesize
            proportionalHeight: r.proportionalHeight
            type: r.type
            typeString: r.typeString ?? ""
            url: r.url
            thumbnailUrl: r.thumbnailUrl
            originalWidth: r.originalWidth
            isOnlyEmoji: r.isOnlyEmoji
            userId: r.userId
            userName: r.userName
            roomTopic: r.roomTopic
            roomName: r.roomName
            callType: r.callType
            relatedEventCacheBuster: r.relatedEventCacheBuster
            encryptionError: r.encryptionError
            // This is disabled so that left clicking the reply goes to its location
            enabled: false
            width: parent.width
            isReply: true
        }

    }

    Rectangle {
        id: backgroundItem

        z: -1
        height: replyContainer.height
        width: Math.min(Math.max(reply.implicitWidth, userName_.implicitWidth) + 8 + 4, parent.width)
        color: Qt.rgba(userColor.r, userColor.g, userColor.b, 0.1)
    }

}
