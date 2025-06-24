#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "albumswindow.h"
#include "alltrackswindow.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>



#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , albumsWindow(nullptr)
    , allTracksWindow(nullptr)
    , playlistsWindow(nullptr)
{
    ui->setupUi(this);
    loadAlbumsFromFile();  


    
    setStyleSheet(R"(
    QMainWindow {
        background-color: #ffe6f0;
    }
    QPushButton {
        background-color: #ff99cc;
        color: white;
        font-size: 20px;
        border-radius: 10px;
    }
    QPushButton:hover {
        background-color: #ff66b2;
    }
)");

    
    connect(ui->btnAlbums, &QPushButton::clicked, this, &MainWindow::openAlbumsWindow);
    connect(ui->btnAllTracks, &QPushButton::clicked, this, &MainWindow::on_btnAllTracks_clicked);
    connect(ui->btnPlaylists, &QPushButton::clicked, this, &MainWindow::on_btnPlaylists_clicked);




}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::openAlbumsWindow() {
    if (!albumsWindow) {
        albumsWindow = new AlbumsWindow();
        connect(albumsWindow, &AlbumsWindow::albumUpdated, this, &MainWindow::updateAlbumInList);

        albumsWindow->setAttribute(Qt::WA_DeleteOnClose);

        
        albumsWindow->setAlbums(albums);

        
        connect(albumsWindow, &AlbumsWindow::windowClosed, this, [this]() {
            this->albums = albumsWindow->getAlbums();  
            this->show();
        });



    }

    albumsWindow->show();
    this->hide();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (albumsWindow) {
        albumsWindow->saveData();  
    }
    QMainWindow::closeEvent(event);
}


void MainWindow::loadAlbumsFromFile() {
    albums.clear(); 

    QFile file("albums.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isArray()) {
            QJsonArray array = doc.array();
            for (const QJsonValue &val : array)
                albums.append(val.toObject());
        }
        file.close();
    }
}



void MainWindow::on_btnPlaylists_clicked()
{
    if (!playlistsWindow) {
        playlistsWindow = new PlaylistsWindow(nullptr, albums);  
        playlistsWindow->setAttribute(Qt::WA_DeleteOnClose);

        
        connect(playlistsWindow, &QObject::destroyed, this, [this]() {
            playlistsWindow = nullptr;
            this->show();
        });
    }

    playlistsWindow->show();  
    this->hide();             
}


void MainWindow::on_btnAllTracks_clicked() {
    if (albums.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Список альбомов пуст. Сначала загрузите альбомы.");
        return;
    }

    if (!allTracksWindow) {
        allTracksWindow = new AllTracksWindow();
        connect(allTracksWindow, &QObject::destroyed, this, [this]() {
            allTracksWindow = nullptr;
            this->show();
        });
    }

    allTracksWindow->loadTracksFromAlbums(albums);
    allTracksWindow->setAttribute(Qt::WA_DeleteOnClose);
    allTracksWindow->show();
    this->hide();
}

void MainWindow::updateAlbumInList(const QJsonObject &updatedAlbum) {
    for (int i = 0; i < albums.size(); ++i) {
        if (albums[i]["title"] == updatedAlbum["title"]) {
            albums[i] = updatedAlbum;
            break;
        }
    }

    
    QFile file("albums.json");
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QJsonArray array;
        for (const QJsonObject &obj : albums)
            array.append(obj);

        QJsonDocument doc(array);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    }
}



