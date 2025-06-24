
#include "addalbumdialog.h"
#include "ui_addalbumdialog.h"
#include <QFileDialog>
#include <QMessageBox>



AddAlbumDialog::AddAlbumDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddAlbumDialog) {
    ui->setupUi(this);

    connect(ui->buttonBoxOk, &QPushButton::clicked, this, &AddAlbumDialog::accept);
    connect(ui->buttonBoxCancel, &QPushButton::clicked, this, &AddAlbumDialog::reject);



    setWindowTitle("Добавить альбом");
    this->setStyleSheet(R"(
    QWidget {
        background-color: #FFE4EC;
        color: #333333;
        font-size: 16px;
    }
    QLabel {
        color: #333333;
    }
    QLineEdit, QSpinBox {
        background-color: white;
        color: #333333;
        border: 1px solid #FF99CC;
        padding: 4px;
        border-radius: 4px;
    }
    QPushButton {
        background-color: #FF99CC;
        color: white;
        font-size: 14px;
        padding: 8px 16px;
        border-radius: 8px;
    }
    QPushButton:hover {
        background-color: #FF66B2;
    }
)");



}

AddAlbumDialog::~AddAlbumDialog() {
    delete ui;
}

QString AddAlbumDialog::getTitle() const {
    return ui->lineEditTitle->text();
}

QString AddAlbumDialog::getArtist() const {
    return ui->lineEditArtist->text();
}

QString AddAlbumDialog::getGenre() const {
    return ui->lineEditGenre->text();
}

int AddAlbumDialog::getYear() const {
    return ui->spinBoxYear->value();
}

QPixmap AddAlbumDialog::getCover() const {
    return albumCover;
}

void AddAlbumDialog::on_loadCover_clicked() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите обложку", "", "Images (*.png *.jpg)");
    if (!filePath.isEmpty()) {
        albumCover.load(filePath);
        ui->labelCover->setPixmap(albumCover.scaled(100, 100, Qt::KeepAspectRatio));
    }
}
void AddAlbumDialog::on_saveButton_clicked() {
    accept();
}

void AddAlbumDialog::on_cancelButton_clicked() {
    reject();
}

