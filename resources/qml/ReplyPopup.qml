// SPDX-FileCopyrightText: 2021 Nheko Contributors
//
// SPDX-License-Identifier: GPL-3.0-or-later

import "./delegates/"
import QtQuick 2.9
import QtQuick.Controls 2.3
import QtQuick.Layouts 1.2
import im.nheko 1.0

Rectangle {
    id: replyPopup

    Layout.fillWidth: true
    visible: room && (room.reply || room.edit)
    // Height of child, plus margins, plus border
    implicitHeight: (room && room.reply ? replyPreview.height : closeEditButton.height) + 10
    color: Nheko.colors.window
    z: 3

    Reply {
        id: replyPreview

        property var modelData: room ? room.getDump(room.reply, room.id) : {
        }

        visible: room && room.reply
        anchors.left: parent.left
        anchors.leftMargin: 2 * 22 + 3 * 16
        anchors.right: closeReplyButton.left
        anchors.rightMargin: 2 * 22 + 3 * 16
        anchors.bottom: parent.bottom
        userColor: TimelineManager.userColor(modelData.userId, Nheko.colors.window)
        blurhash: modelData.blurhash ?? ""
        body: modelData.body ?? ""
        formattedBody: modelData.formattedBody ?? ""
        eventId: modelData.eventId ?? ""
        filename: modelData.filename ?? ""
        filesize: modelData.filesize ?? ""
        proportionalHeight: modelData.proportionalHeight ?? 1
        type: modelData.type ?? MtxEvent.UnknownMessage
        typeString: modelData.typeString ?? ""
        url: modelData.url ?? ""
        originalWidth: modelData.originalWidth ?? 0
        isOnlyEmoji: modelData.isOnlyEmoji ?? false
        userId: modelData.userId ?? ""
        userName: modelData.userName ?? ""
        encryptionError: modelData.encryptionError ?? ""
    }

    ImageButton {
        id: closeReplyButton

        visible: room && room.reply
        anchors.right: parent.right
        anchors.rightMargin: 16
        anchors.top: replyPreview.top
        hoverEnabled: true
        width: 16
        height: 16
        image: ":/icons/icons/ui/dismiss.svg"
        ToolTip.visible: closeReplyButton.hovered
        ToolTip.text: qsTr("Close")
        onClicked: room.reply = undefined
    }

    Button {
        id: closeEditButton

        visible: room && room.edit
        anchors.left: parent.left
        anchors.rightMargin: 16
        anchors.topMargin: 10
        anchors.top: parent.top
        //height: 16
        text: qsTr("Cancel edit")
        onClicked: room.edit = undefined
    }

}
