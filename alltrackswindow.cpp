#include "alltrackswindow.h"
#include "ui_alltrackswindow.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QFile>
#include <QMessageBox>
#include <QInputDialog>
#include <QTimer>



AllTracksWindow::AllTracksWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::AllTracksWindow)    ,

    model(new QStandardItemModel(this))  
{
    ui->setupUi(this);

    ui->tableViewTracks->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);


    resize(900, 700); 
      
    this->setStyleSheet(R"(
    QWidget {
        background-color: #FFE4EC;
        color: #333;
        font-size: 16px;
    }

    QTableView {
        background-color: #FFF0F5;
        gridline-color: #FF99CC;
        selection-background-color: #FF99CC;
        selection-color: white;
    }

    QTableView::item {
        background-color: #FFE4EC;
        color: #333;
    }

    QHeaderView::section {
        background-color: #FF99CC;
        color: white;
        font-weight: bold;
        border: 1px solid #FF66B2;
    }

    QPushButton {
        background-color: #FF99CC;
        color: white;
        border-radius: 8px;
        padding: 6px;
    }

    QPushButton:hover {
        background-color: #FF66B2;
    }
)");




    model->setColumnCount(6); 
    model->setHeaderData(0, Qt::Horizontal, "Название");
    model->setHeaderData(1, Qt::Horizontal, "Исполнитель");
    model->setHeaderData(2, Qt::Horizontal, "Жанр");
    model->setHeaderData(3, Qt::Horizontal, "Год");
    model->setHeaderData(4, Qt::Horizontal, "Длительность");
    model->setHeaderData(5, Qt::Horizontal, "Путь к файлу"); 

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1);

    ui->tableViewTracks->setModel(proxyModel);
    ui->tableViewTracks->setColumnHidden(5, true); 
    ui->tableViewTracks->setSortingEnabled(true);
    ui->tableViewTracks->horizontalHeader()->setStretchLastSection(true);
    ui->tableViewTracks->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewTracks->setSelectionMode(QAbstractItemView::SingleSelection); 


    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, [=](const QString &text) {
        proxyModel->setFilterRegularExpression(QRegularExpression(text, QRegularExpression::CaseInsensitiveOption));
    });

    connect(ui->comboBoxFilter, &QComboBox::currentIndexChanged, this, [=](int index) {
        int column = index - 1;
        proxyModel->setFilterKeyColumn(column >= 0 ? column : -1);
    });

    connect(ui->tableViewTracks, &QTableView::doubleClicked,
            this, &AllTracksWindow::onTrackDoubleClicked);
    connect(ui->btnAddToPlaylist, &QPushButton::clicked,
            this, &AllTracksWindow::onAddToPlaylistClicked);

}


AllTracksWindow::~AllTracksWindow()
{
    delete ui;
}

void AllTracksWindow::loadTracksFromAlbums(const QList<QJsonObject> &albums)
{
    model->removeRows(0, model->rowCount());

    for (const QJsonObject &album : albums) {
        if (!album.contains("tracks") || !album["tracks"].isArray())
            continue;

        QJsonArray tracks = album["tracks"].toArray();
        for (const QJsonValue &val : tracks) {
            if (!val.isObject()) continue;

            QJsonObject track = val.toObject();
            QList<QStandardItem*> row;
            row << new QStandardItem(track["title"].toString());
            row << new QStandardItem(track["artist"].toString());
            row << new QStandardItem(track["genre"].toString());
            row << new QStandardItem(track["year"].toString());
            row << new QStandardItem(track["duration"].toString());
            row << new QStandardItem(track["filePath"].toString()); 

            for (auto *item : row)
                item->setTextAlignment(Qt::AlignCenter);

            model->appendRow(row);
        }
    }

    
    proxyModel->setSourceModel(model);
    ui->tableViewTracks->setModel(proxyModel); 


    
    ui->tableViewTracks->hideColumn(5);

    
    ui->tableViewTracks->setColumnWidth(0, 300);
    ui->tableViewTracks->setColumnWidth(1, 150);
    ui->tableViewTracks->setColumnWidth(2, 120);
    ui->tableViewTracks->setColumnWidth(3, 100);
    ui->tableViewTracks->setColumnWidth(4, 70);
}


void AllTracksWindow::playTrack(const Track &track) {
    
    

    QList<Track> allTracks;
    for (int i = 0; i < model->rowCount(); ++i) {
        Track t;
        t.title = model->item(i, 0)->text();
        t.artist = model->item(i, 1)->text();
        t.genre = model->item(i, 2)->text();
        t.year = model->item(i, 3)->text();
        t.duration = model->item(i, 4)->text();
        t.filePath = model->item(i, 5)->text();
        allTracks.append(t);
    }

    
    int selectedIndex = proxyModel->mapToSource(ui->tableViewTracks->currentIndex()).row();

    if (!playerWindow)
        playerWindow = new PlayerWindow(this);

    playerWindow->setTrackList(allTracks, selectedIndex);
    playerWindow->show();
}




void AllTracksWindow::onTrackDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    int row = index.row();
    QModelIndex sourceIndex = proxyModel->mapToSource(proxyModel->index(row, 0));

    QString title = model->item(sourceIndex.row(), 0)->text();
    QString artist = model->item(sourceIndex.row(), 1)->text();
    QString genre = model->item(sourceIndex.row(), 2)->text();
    QString year = model->item(sourceIndex.row(), 3)->text();
    QString duration = model->item(sourceIndex.row(), 4)->text();

    QString filePath = model->item(sourceIndex.row(), 5)->text(); 

    if (filePath.isEmpty() || !QFile::exists(filePath)) {
        QMessageBox::warning(this, "Ошибка", "Файл трека отсутствует или не найден:\n" + filePath);
        return;
    }

    Track track{title, duration, filePath, artist, genre, year};
    playTrack(track);

}
void AllTracksWindow::onAddToPlaylistClicked()
{
    QModelIndexList selected = ui->tableViewTracks->selectionModel()->selectedRows();
    if (selected.isEmpty()) {
        QMessageBox::warning(this, "Нет выбора", "Выберите хотя бы один трек.");
        return;
    }

    QList<QJsonObject> selectedTracks;
    for (const QModelIndex &proxyIndex : selected) {
        int row = proxyModel->mapToSource(proxyIndex).row();

        QJsonObject track;
        track["title"] = model->item(row, 0)->text();
        track["artist"] = model->item(row, 1)->text();
        track["genre"] = model->item(row, 2)->text();
        track["year"] = model->item(row, 3)->text();
        track["duration"] = model->item(row, 4)->text();
        track["filePath"] = model->item(row, 5)->text();

        selectedTracks.append(track);
    }

    
    if (receivers(SIGNAL(tracksSelectedForPlaylist(QList<QJsonObject>))) > 0) {
        emit tracksSelectedForPlaylist(selectedTracks);
        close();
        return;
    }

    
    QFile file("playlists.json");
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть playlists.json");
        return;
    }

    QJsonArray playlists;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    if (doc.isArray()) playlists = doc.array();
    file.close();

    QStringList names;
    for (const QJsonValue &val : playlists) {
        QJsonObject obj = val.toObject();
        names << obj["title"].toString();
    }

    bool ok;
    QString chosen = QInputDialog::getItem(this, "Выбор плейлиста", "Добавить в плейлист:", names, 0, false, &ok);
    if (!ok || chosen.isEmpty()) return;

    for (int i = 0; i < playlists.size(); ++i) {
        QJsonObject pl = playlists[i].toObject();
        if (pl["title"].toString() == chosen) {
            QJsonArray arr = pl["tracks"].toArray();
            for (const QJsonObject &t : selectedTracks)
                arr.append(t);
            pl["tracks"] = arr;
            playlists[i] = pl;
            break;
        }
    }

    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить playlists.json");
        return;
    }

    file.write(QJsonDocument(playlists).toJson());
    file.close();

    QMessageBox::information(this, "Готово", "Треки добавлены в плейлист.");
}

