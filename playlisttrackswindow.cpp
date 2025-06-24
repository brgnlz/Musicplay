#include "playlisttrackswindow.h"
#include "ui_playlisttrackswindow.h"
#include "alltrackswindow.h"

#include <QJsonArray>
#include <QMessageBox>
#include <QFile>
#include <QPushButton>




#include "playlisttrackswindow.h"
#include "ui_playlisttrackswindow.h"

PlaylistTracksWindow::PlaylistTracksWindow(QWidget *parent, const QList<QJsonObject> &albumsList)
    : QDialog(parent), ui(new Ui::PlaylistTracksWindow), albums(albumsList)
{
    ui->setupUi(this);
    model = new QStandardItemModel(this);
    ui->tableView->setMaximumWidth(750);
    ui->tableView->setMinimumWidth(600);
    ui->tableView->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    ui->tableView->verticalHeader()->setVisible(false);

    ui->tableView->setStyleSheet(R"(
    QTableView {
        background-color: white;
        gridline-color: #FF99CC;
        selection-background-color: #ff99cc;
        selection-color: white;
        color: black; 
    }

    QTableView::item:selected {
        background-color: #ff99cc;
        color: white;
    }
)");


    ui->tableView->setStyleSheet(R"(
    QTableView {
        background-color: #ffeaf4;  
        color: #333;                
        gridline-color: #FFCCE5;
        selection-background-color: #FF99CC;
        selection-color: black;
    }

    QTableView::item {
        padding: 6px;
        border: none;
    }

    QTableView::item:selected {
        background-color: #FF99CC;
        color: black;
    }

    QHeaderView::section {
        background-color: #FF99CC;
        color: white;
        font-weight: bold;
        padding: 6px;
        border: none;
    }
)");



    setFixedSize(900, 700);
    setWindowTitle("Треки плейлиста");

    model->setColumnCount(5);
    model->setHorizontalHeaderLabels({"Название", "Исполнитель", "Жанр", "Год", "Длительность"});
    ui->tableView->setModel(model);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    connect(ui->tableView, &QTableView::doubleClicked,
            this, &PlaylistTracksWindow::onTrackDoubleClicked);
    connect(ui->btnPlay, &QPushButton::clicked, this, &PlaylistTracksWindow::on_btnPlay_clicked);
    connect(ui->btnDelete, &QPushButton::clicked, this, &PlaylistTracksWindow::on_btnDelete_clicked);
    connect(ui->btnAddTrack, &QPushButton::clicked, this, &PlaylistTracksWindow::on_btnAddTrack_clicked);


}

PlaylistTracksWindow::~PlaylistTracksWindow()
{
    delete ui;
}

void PlaylistTracksWindow::setPlaylist(const QJsonObject &pl)
{
    if (!pl.contains("title")) {
        qDebug() << "[Ошибка] Плейлист не содержит поля title!";
        return;
    }

    playlist = pl;
    refreshUI();  
}


void PlaylistTracksWindow::loadTracks()
{
    if (!playlist.contains("tracks")) return;

    QJsonArray tracks = playlist["tracks"].toArray();

    for (const QJsonValue &val : tracks) {
        if (!val.isObject()) continue;

        QJsonObject t = val.toObject();
        QList<QStandardItem*> row;
        row << new QStandardItem(t["title"].toString());
        row << new QStandardItem(t["artist"].toString());
        row << new QStandardItem(t["genre"].toString());
        row << new QStandardItem(t["year"].toString());
        row << new QStandardItem(t["duration"].toString());

        for (auto *item : row)
            item->setTextAlignment(Qt::AlignCenter);

        model->appendRow(row);
    }
}

void PlaylistTracksWindow::onTrackDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QJsonArray tracks = playlist["tracks"].toArray();  
    if (row >= tracks.size()) return;

    QJsonObject track = tracks[row].toObject();
    QString path = track["filePath"].toString();
    if (path.isEmpty() || !QFile::exists(path)) {
        QMessageBox::warning(this, "Ошибка", "Файл трека не найден:\n" + path);
        return;
    }

    Track t{
        track["title"].toString(),
        track["duration"].toString(),
        path,
        track["artist"].toString(),
        track["genre"].toString(),
        track["year"].toString()
    };

    if (!playerWindow) playerWindow = new PlayerWindow();
    QJsonArray jsonTracks = playlist["tracks"].toArray();
    QList<Track> trackList;
    for (const QJsonValue &val : jsonTracks) {
        QJsonObject obj = val.toObject();
        Track t {
            obj["title"].toString(),
            obj["duration"].toString(),
            obj["filePath"].toString(),
            obj["artist"].toString(),
            obj["genre"].toString(),
            obj["year"].toString()
        };
        trackList.append(t);
    }

    int selectedIndex = index.row();

    if (!playerWindow) playerWindow = new PlayerWindow(this);
    playerWindow->setTrackList(trackList, selectedIndex);
    playerWindow->show();

    playerWindow->show();
}

void PlaylistTracksWindow::addTracks(const QList<QJsonObject> &tracks)
{
    QJsonArray current = playlist["tracks"].toArray();
    for (const auto &track : tracks)
        current.append(track);

    playlist["tracks"] = current;
    
}
void PlaylistTracksWindow::refreshUI()
{
    ui->labelTitle->setText("<h1 style='color:#FF66B2;'>" + playlist["title"].toString() + "</h1>");
    ui->labelDescription->setText("<p style='font-size:14pt; color:#FF99CC;'>" + playlist["description"].toString() + "</p>");


    QString coverPath = playlist["cover"].toString();
    QPixmap pix;
    if (!coverPath.isEmpty() && QFile::exists(coverPath) && pix.load(coverPath)) {
        ui->labelCover->setPixmap(pix.scaled(150, 150, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    } else {
        ui->labelCover->clear();  
    }



    ui->labelCover->setAlignment(Qt::AlignCenter);

    model->clear();
    model->setColumnCount(5);
    model->setHorizontalHeaderLabels(QStringList()
                                     << "Название" << "Исполнитель" << "Жанр" << "Год" << "Длительность");

    QJsonArray tracks = playlist["tracks"].toArray();

    for (const QJsonValue &val : tracks) {
        if (!val.isObject()) continue;
        QJsonObject obj = val.toObject();

        QList<QStandardItem*> row;
        row << new QStandardItem(obj["title"].toString());
        row << new QStandardItem(obj["artist"].toString());
        row << new QStandardItem(obj["genre"].toString());
        row << new QStandardItem(obj["year"].toString());
        row << new QStandardItem(obj["duration"].toString());

        for (auto *item : row)
            item->setTextAlignment(Qt::AlignCenter);

        model->appendRow(row);
    }

    ui->tableView->setModel(model);
    ui->tableView->horizontalHeader()->setStretchLastSection(false);
    ui->tableView->resizeColumnsToContents();
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

void PlaylistTracksWindow::on_btnPlay_clicked()
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = index.row();
    QJsonArray tracks = playlist["tracks"].toArray();
    if (row >= tracks.size()) return;

    QJsonObject track = tracks[row].toObject();
    QString filePath = track["filePath"].toString();
    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        QMessageBox::warning(this, "Ошибка", "Файл не найден:\n" + filePath);
        return;
    }

    Track t {
        track["title"].toString(),
        track["duration"].toString(),
        filePath,
        track["artist"].toString(),
        track["genre"].toString(),
        track["year"].toString()
    };

    if (!playerWindow) playerWindow = new PlayerWindow();
    QJsonArray jsonTracks = playlist["tracks"].toArray();
    QList<Track> trackList;
    for (const QJsonValue &val : jsonTracks) {
        QJsonObject obj = val.toObject();
        Track t {
            obj["title"].toString(),
            obj["duration"].toString(),
            obj["filePath"].toString(),
            obj["artist"].toString(),
            obj["genre"].toString(),
            obj["year"].toString()
        };
        trackList.append(t);
    }

    int selectedIndex = index.row();

    if (!playerWindow) playerWindow = new PlayerWindow(this);
    playerWindow->setTrackList(trackList, selectedIndex);
    playerWindow->show();

    playerWindow->show();
}
void PlaylistTracksWindow::on_btnDelete_clicked()
{
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = index.row();
    QJsonArray tracks = playlist["tracks"].toArray();

    if (row >= 0 && row < tracks.size()) {
        tracks.removeAt(row);
        playlist["tracks"] = tracks;

        
        QFile file("playlists.json");
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            if (doc.isArray()) {
                QJsonArray allPlaylists = doc.array();
                for (int i = 0; i < allPlaylists.size(); ++i) {
                    if (allPlaylists[i].toObject()["title"] == playlist["title"]) {
                        allPlaylists[i] = playlist;
                        break;
                    }
                }

                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    file.write(QJsonDocument(allPlaylists).toJson());
                    file.close();
                }
            }
        }

        refreshUI();
    }
}

void PlaylistTracksWindow::on_btnAddTrack_clicked()
{
    AllTracksWindow *selector = new AllTracksWindow(this);
    selector->loadTracksFromAlbums(albums);

    connect(selector, &AllTracksWindow::tracksSelectedForPlaylist, this, [=](const QList<QJsonObject> &selectedTracks) {
        QJsonArray current = playlist["tracks"].toArray();
        for (const QJsonObject &track : selectedTracks)
            current.append(track);

        playlist["tracks"] = current;

        
        QFile file("playlists.json");
        if (file.open(QIODevice::ReadOnly)) {
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
            file.close();

            if (doc.isArray()) {
                QJsonArray all = doc.array();
                for (int i = 0; i < all.size(); ++i) {
                    if (all[i].toObject()["title"] == playlist["title"]) {
                        all[i] = playlist;
                        break;
                    }
                }
                if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
                    file.write(QJsonDocument(all).toJson());
                    file.close();
                }
            }
        }

        refreshUI();
    });

    selector->setWindowModality(Qt::ApplicationModal);  
    selector->show();  
      
}




void PlaylistTracksWindow::setAlbums(const QList<QJsonObject> &albumList) {
    albums = albumList;
}
