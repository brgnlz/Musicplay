#include "playlisteditdialog.h"
#include "ui_playlisteditdialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QPixmap>

PlaylistEditDialog::PlaylistEditDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PlaylistEditDialog)
{
    ui->setupUi(this);
    setStyleSheet(R"(
    QDialog {
        background-color: #ffe6f0;
    }

    QPushButton {
        background-color: #ff99cc;
        color: white;
        border-radius: 10px;
        height: 36px;
        font-weight: bold;
    }

    QPushButton:hover {
        background-color: #ff66b2;
    }

    QLineEdit, QTextEdit {
        background-color: white;
        border: 1px solid #ccc;
        padding: 5px;
        border-radius: 5px;
        color: #333;
    }

    QLabel {
        color: #2e2e2e;
    }
)");
    this->setMinimumSize(400, 400);

    setWindowTitle("Новый плейлист");

}
void PlaylistEditDialog::on_btnSelectCover_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Выбрать обложку", "", "Images (*.png *.jpg *.jpeg)");
    if (!filePath.isEmpty()) {
        coverPath = filePath;
        ui->btnSelectCover->setText("✅ Обложка выбрана");
    }
}



void PlaylistEditDialog::on_btnOk_clicked()
{
    if (ui->lineEditTitle->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите название плейлиста.");
        return;
    }
    accept();
}

void PlaylistEditDialog::on_btnCancel_clicked()
{
    reject();
}

void PlaylistEditDialog::setData(const QJsonObject &playlist)
{
    ui->lineEditTitle->setText(playlist["title"].toString());
    ui->textEditDescription->setText(playlist["description"].toString());
    coverPath = playlist["cover"].toString();

    if (!coverPath.isEmpty()) {
        QPixmap pixmap(coverPath);
        ui->labelCoverPreview->setPixmap(pixmap.scaled(100, 100, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

QJsonObject PlaylistEditDialog::getData() const
{
    QJsonObject obj;
    obj["title"] = ui->lineEditTitle->text().trimmed();
    obj["description"] = ui->textEditDescription->toPlainText().trimmed();
    obj["cover"] = coverPath;
    return obj;
}
