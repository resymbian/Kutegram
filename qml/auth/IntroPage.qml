import QtQuick 1.0
import "../control"

Rectangle {
    id: introPageRect

    Column {
        anchors.centerIn: parent
        width: parent.width * 2 / 3
        spacing: 5

        Rectangle {
            anchors.horizontalCenter: parent.horizontalCenter
            width: Math.min(introPageRect.height / 2, parent.width * 2 / 3)
            height: width
            color: globalAccent
            radius: width / 4

            Image {
                anchors.fill: parent
                //asynchronous: true
                smooth: true
                source: "../../kutegramquick_big.png"
            }
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Kutegram"
            font.bold: true
            wrapMode: Text.Wrap
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 12 * kgScaling
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Just another unofficial Telegram client."
            wrapMode: Text.Wrap
            font.pixelSize: 12 * kgScaling
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Telegram API ID / hash"
            font.bold: true
            wrapMode: Text.Wrap
            font.pixelSize: 12 * kgScaling
        }

        Text {
            anchors.left: parent.left
            anchors.right: parent.right
            text: "Create an app at my.telegram.org/apps and enter its ID and hash once. They are stored on this device only."
            wrapMode: Text.Wrap
            font.pixelSize: 12 * kgScaling
        }

        LineEdit {
            id: apiIdEdit
            anchors.left: parent.left
            anchors.right: parent.right
            text: apiSettings.apiId > 0 ? apiSettings.apiId.toString() : ""
        }

        LineEdit {
            id: apiHashEdit
            anchors.left: parent.left
            anchors.right: parent.right
            text: apiSettings.apiHash
        }

        Button {
            anchors.left: parent.left
            anchors.right: parent.right
            enabled: !root.authProgress
            onClicked: {
                var id = parseInt(apiIdEdit.text, 10);
                var hash = apiHashEdit.text.replace(/\s/g, "");
                if (id <= 0 || hash.length < 20) {
                    snackBar.text = "Enter the API ID and hash from my.telegram.org/apps.";
                    return;
                }

                apiSettings.setCredentials(id, hash);
                snackBar.close();
                telegramClient.resetSession();
                root.setAuthProgress(true);
                telegramClient.start();
            }
        }
    }
}
