#include "albumtrackswindow.h"
#include "ui_albumtrackswindow.h"
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QSortFilterProxyModel>  


#include <QFileDialog>
#include <QMediaPlayer>
#include <QMediaMetaData>
#include <QAudioOutput>
#include <QEventLoop>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QPixmap>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>



AlbumTracksWindow::AlbumTracksWindow(const QJsonObject& album, QWidget *parent)
    : QWidget(parent),
    ui(new Ui::AlbumTracksWindow),
    albumData(album),
    model(new QStandardItemModel(this))
{
    ui->setupUi(this);
    this->setWindowFlags(Qt::Window); 
    this->resize(800, 600);           
    this->setMinimumSize(600, 400);
    ui->labelCover->setMaximumSize(200, 200);
    ui->labelCover->setScaledContents(true);  

    ui->labelTitle->setText(album["title"].toString());
    setWindowTitle(album["title"].toString());

    
    QByteArray imageData = QByteArray::fromBase64(album["cover"].toString().toLatin1());
    QPixmap pixmap;
    pixmap.loadFromData(imageData, "PNG");
    ui->labelCover->setPixmap(pixmap.scaled(200, 200, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));

    
    model->setColumnCount(5);
    model->setHeaderData(0, Qt::Horizontal, "Название");
    model->setHeaderData(1, Qt::Horizontal, "Длительность");
    model->setHeaderData(2, Qt::Horizontal, "Исполнитель");
    model->setHeaderData(3, Qt::Horizontal, "Жанр");
    model->setHeaderData(4, Qt::Horizontal, "Год");


    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    loadAlbumTracks();
    updateTable();

    connect(ui->btnAddTrack, &QPushButton::clicked, this, &AlbumTracksWindow::onAddTrack);
    connect(ui->btnEditTrack, &QPushButton::clicked, this, &AlbumTracksWindow::onEditTrack);
    connect(ui->btnDeleteTrack, &QPushButton::clicked, this, &AlbumTracksWindow::onDeleteTrack);
    connect(ui->btnPlayTrack, &QPushButton::clicked, this, &AlbumTracksWindow::onPlayTrack);


}

AlbumTracksWindow::~AlbumTracksWindow()
{
    saveAlbumTracks();
    delete ui;
}

void AlbumTracksWindow::loadAlbumTracks() {
    QFile file("albums.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть albums.json для чтения";
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        qWarning() << "albums.json не является массивом";
        return;
    }

    QJsonArray albumArray = doc.array();
    QString currentTitle = albumData["title"].toString();

    trackList.clear();

    for (const QJsonValue& val : albumArray) {
        QJsonObject obj = val.toObject();
        if (obj["title"].toString() == currentTitle) {
            QJsonArray trackArray = obj["tracks"].toArray();
            for (const QJsonValue& trackVal : trackArray) {
                QJsonObject trackObj = trackVal.toObject();
                Track t;
                t.title = trackObj["title"].toString();
                t.duration = trackObj["duration"].toString();
                t.filePath = trackObj["filePath"].toString();
                t.artist = trackObj["artist"].toString();
                t.genre = trackObj["genre"].toString();
                t.year = trackObj["year"].toString();
                trackList.append(t);
            }
            break;
        }
    }

    qDebug() << "Загружено треков:" << trackList.size();
}


void AlbumTracksWindow::saveAlbumTracks() {
    qDebug() << "Сохраняем треки...";

    QFile file("albums.json");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Не удалось открыть albums.json для чтения";
        return;
    }

    QByteArray jsonData = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (!doc.isArray()) {
        qWarning() << "albums.json не является массивом";
        return;
    }

    QJsonArray albumArray = doc.array();
    QString albumTitle = ui->labelTitle->text();
    bool updated = false;

    QJsonObject updatedAlbum;  

    for (int i = 0; i < albumArray.size(); ++i) {
        QJsonObject obj = albumArray[i].toObject();
        if (obj["title"].toString() == albumTitle) {
            updatedAlbum = obj;

            QJsonArray trackArray;
            for (const Track& t : trackList) {
                QJsonObject trackObj;
                trackObj["title"] = t.title;
                trackObj["duration"] = t.duration;
                trackObj["filePath"] = t.filePath;
                trackObj["artist"] = t.artist;
                trackObj["genre"] = t.genre;
                trackObj["year"] = t.year;
                trackArray.append(trackObj);
            }

            updatedAlbum["tracks"] = trackArray;
            albumArray[i] = updatedAlbum;
            updated = true;
            break;

        }
    }

    if (!updated) {
        qWarning() << "Альбом не найден";
        return;
    }

    QFile outFile("albums.json");
    if (!outFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qWarning() << "Не удалось открыть albums.json для записи";
        return;
    }

    QJsonDocument outDoc(albumArray);
    outFile.write(outDoc.toJson(QJsonDocument::Indented));
    outFile.close();

    qDebug() << "Треки успешно сохранены.";

    emit albumUpdated(updatedAlbum);  
    albumData = updatedAlbum;  

}







void AlbumTracksWindow::onAddTrack() {
    QString filePath = QFileDialog::getOpenFileName(this, "Выберите MP3", "", "MP3 (*.mp3)");
    if (filePath.isEmpty()) return;

    QMediaPlayer metaPlayer;
    QAudioOutput audioOut;
    metaPlayer.setAudioOutput(&audioOut);
    metaPlayer.setSource(QUrl::fromLocalFile(filePath));

    QEventLoop loop;
    connect(&metaPlayer, &QMediaPlayer::mediaStatusChanged, &loop, [&](QMediaPlayer::MediaStatus status) {
        if (status == QMediaPlayer::LoadedMedia)
            loop.quit();
    });
    loop.exec();

    
    QMediaMetaData meta = metaPlayer.metaData();

    QString title = meta.stringValue(QMediaMetaData::Title);
    if (title.isEmpty())
        title = QFileInfo(filePath).baseName();

    qint64 durationMs = metaPlayer.duration();
    int seconds = durationMs / 1000;
    QString durationStr = QString("%1:%2")
                              .arg(seconds / 60)
                              .arg(seconds % 60, 2, 10, QChar('0'));

    Track newTrack;
    newTrack.title = title;
    newTrack.duration = durationStr;
    newTrack.filePath = filePath;
    newTrack.artist = albumData["artist"].toString();
    newTrack.genre = albumData["genre"].toString();
    newTrack.year = albumData["year"].toString();

    trackList.append(newTrack);
    updateTable();
}


void AlbumTracksWindow::onEditTrack() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = index.row();
    if (row < 0 || row >= trackList.size()) return;

    Track &track = trackList[row];

    bool ok;
    QString newTitle = QInputDialog::getText(this, "Редактировать трек",
                                             "Название:", QLineEdit::Normal, track.title, &ok);
    if (!ok || newTitle.isEmpty()) return;

    track.title = newTitle;

    

    updateTable();
}

void AlbumTracksWindow::onDeleteTrack() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = index.row();
    if (row < 0 || row >= trackList.size()) return;

    int confirm = QMessageBox::question(this, "Удалить трек",
                                        "Вы уверены, что хотите удалить этот трек?");
    if (confirm == QMessageBox::Yes) {
        trackList.removeAt(row);
        updateTable();
    }
}

void AlbumTracksWindow::updateTable() {
    model->clear();
    model->setColumnCount(5);
    model->setHeaderData(0, Qt::Horizontal, "Название");
    model->setHeaderData(1, Qt::Horizontal, "Исполнитель");
    model->setHeaderData(2, Qt::Horizontal, "Жанр");
    model->setHeaderData(3, Qt::Horizontal, "Год");
    model->setHeaderData(4, Qt::Horizontal, "Длительность");

    for (const Track& t : trackList) {
        QList<QStandardItem*> row;
        row << new QStandardItem(t.title);
        row << new QStandardItem(t.artist);
        row << new QStandardItem(t.genre);
        row << new QStandardItem(t.year);
        row << new QStandardItem(t.duration);
        model->appendRow(row);
    }
    
    ui->tableView->setColumnWidth(0, 220);  
    ui->tableView->setColumnWidth(1, 150);  
    ui->tableView->setColumnWidth(2, 120);  
    ui->tableView->setColumnWidth(3, 80);   
    ui->tableView->setColumnWidth(4, 80);   

}

void AlbumTracksWindow::closeEvent(QCloseEvent *event)
{
    saveAlbumTracks();  

    emit albumUpdated(albumData);     
    emit requestUpdateAllTracks();    

    QWidget::closeEvent(event);
}


void AlbumTracksWindow::onPlayTrack() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid())
        return;

    int selectedIndex = index.row();  

    if (!playerWindow)
        playerWindow = new PlayerWindow(this);

    playerWindow->setTrackList(trackList, selectedIndex);
    playerWindow->show();
}


