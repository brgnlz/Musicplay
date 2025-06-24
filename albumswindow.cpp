#include "albumswindow.h"
#include "addalbumdialog.h"
#include "playerwindow.h"
#include "track.h"
#include "ui_albumswindow.h"
#include "albumtrackswindow.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QFile>
#include <QBuffer>
#include <QPixmap>
#include <QPainter>


AlbumsWindow::AlbumsWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::AlbumsWindow)
{
    ui->setupUi(this);

    
    ui->comboBoxFilter->addItem("Все поля", -1);
    ui->comboBoxFilter->addItem("Название", 1);
    ui->comboBoxFilter->addItem("Исполнитель", 2);
    ui->comboBoxFilter->addItem("Жанр", 3);
    ui->comboBoxFilter->addItem("Год выпуска", 4);

    
    ui->tableView->setIconSize(QSize(100, 100));
    ui->tableView->verticalHeader()->setDefaultSectionSize(110);
    ui->tableView->setColumnWidth(0, 110);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    
    setupModel();

    
    setFixedSize(900, 700);
    this->setStyleSheet(R"(
        QWidget {
            background-color: #FFE4EC;
            color: #333;
            font-size: 16px;
        }

        QPushButton {
            background-color: #FF99CC;
            color: white;
            padding: 10px;
            font-size: 16px;
            border: none;
            border-radius: 8px;
        }

        QPushButton:hover {
            background-color: #FF66B2;
        }

        QHeaderView::section {
            background-color: #FF99CC;
            color: white;
            font-weight: bold;
            padding: 4px;
            border: 1px solid #FF66B2;
        }

        QTableView {
            background-color: white;
            gridline-color: #FF99CC;
            selection-background-color: #FFCCE5;
            selection-color: black;
        }
    )");

    
    connect(ui->btnOpenAlbum, &QPushButton::clicked, this, &AlbumsWindow::onOpenAlbumClicked);
    connect(ui->tableView, &QTableView::doubleClicked, this, &AlbumsWindow::onAlbumDoubleClicked);
    connect(ui->btnAdd, &QPushButton::clicked, this, &AlbumsWindow::on_addAlbumButton_clicked);
    connect(ui->btnEdit, &QPushButton::clicked, this, &AlbumsWindow::editAlbum);
    connect(ui->btnDelete, &QPushButton::clicked, this, &AlbumsWindow::deleteAlbum);
}

AlbumsWindow::~AlbumsWindow()
{
    delete ui;
}

void AlbumsWindow::setupModel() {
    model = new QStandardItemModel(this);
    model->setHorizontalHeaderLabels({"Обложка", "Название", "Исполнитель", "Жанр", "Год выпуска"});

    proxyModel = new QSortFilterProxyModel(this);
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    proxyModel->setFilterKeyColumn(-1);

    ui->tableView->setModel(proxyModel);
    ui->tableView->setSortingEnabled(true);

    
    connect(ui->lineEditSearch, &QLineEdit::textChanged, this, [=](const QString &text) {
        int column = ui->comboBoxFilter->currentIndex() - 1;
        proxyModel->setFilterKeyColumn(column >= 0 ? column : -1);
        proxyModel->setFilterFixedString(text);
    });

    
    connect(ui->comboBoxFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        int column = index - 1;
        proxyModel->setFilterKeyColumn(column >= 0 ? column : -1);
        proxyModel->setFilterFixedString(ui->lineEditSearch->text());
    });

    loadData();
}





void AlbumsWindow::editAlbum() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    QString currentValue = index.data().toString();
    QString newValue = QInputDialog::getText(this, "Редактировать", "Новое значение:", QLineEdit::Normal, currentValue);

    if (!newValue.isEmpty()) {
        model->setData(proxyModel->mapToSource(index), newValue);
    }
}

void AlbumsWindow::deleteAlbum() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    model->removeRow(proxyModel->mapToSource(index).row());
}


void AlbumsWindow::on_addAlbumButton_clicked()
{
    AddAlbumDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        QString title = dialog.getTitle();
        QString artist = dialog.getArtist();
        QString genre = dialog.getGenre();
        int year = dialog.getYear();
        QPixmap cover = dialog.getCover();

        
        if (cover.isNull()) {
            cover = QPixmap(100, 100);
            cover.fill(QColor("#FF99CC")); 

            QPainter painter(&cover);
            painter.setPen(Qt::white);
            painter.setFont(QFont("Arial", 12, QFont::Bold));
            painter.drawText(cover.rect(), Qt::AlignCenter, "Без\nобложки");
            painter.end();
        }

        
        QPixmap scaled = cover.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

        QStandardItem* itemCover = new QStandardItem();
        itemCover->setData(QVariant(scaled), Qt::DecorationRole);
        itemCover->setEditable(false);
        itemCover->setText("");
        itemCover->setTextAlignment(Qt::AlignCenter);

        QStandardItem* itemTitle = new QStandardItem(title);
        itemTitle->setTextAlignment(Qt::AlignCenter);
        QStandardItem* itemArtist = new QStandardItem(artist);
        itemArtist->setTextAlignment(Qt::AlignCenter);
        QStandardItem* itemGenre = new QStandardItem(genre);
        itemGenre->setTextAlignment(Qt::AlignCenter);
        QStandardItem* itemYear = new QStandardItem(QString::number(year));
        itemYear->setTextAlignment(Qt::AlignCenter);

        QList<QStandardItem*> row;
        row << itemCover << itemTitle << itemArtist << itemGenre << itemYear;
        model->appendRow(row);

        
        QJsonObject album;
        album["title"] = title;
        album["artist"] = artist;
        album["genre"] = genre;
        album["year"] = QString::number(year);

        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        cover.save(&buffer, "PNG");
        album["cover"] = QString::fromLatin1(bytes.toBase64());

        albums.append(album);

        ui->tableView->setIconSize(QSize(100, 100));
        ui->tableView->verticalHeader()->setDefaultSectionSize(110);
        ui->tableView->setColumnWidth(0, 110);
    }
}


void AlbumsWindow::closeEvent(QCloseEvent *event)
{
    saveData();  
    emit windowClosed();         
    QMainWindow::closeEvent(event);
}

QList<QJsonObject> AlbumsWindow::getAlbums() const {
    return albums;
}


void AlbumsWindow::saveData()
{
    QJsonArray albumArray;

    for (int row = 0; row < model->rowCount(); ++row)
    {
        QJsonObject album;

        QString title = model->item(row, 1)->text();  
        album["title"] = title;
        album["artist"] = model->item(row, 2)->text();
        album["genre"] = model->item(row, 3)->text();
        album["year"] = model->item(row, 4)->text();

        
        QVariant decoration = model->item(row, 0)->data(Qt::DecorationRole);
        if (decoration.canConvert<QPixmap>()) {
            QPixmap pixmap = decoration.value<QPixmap>();
            QByteArray bytes;
            QBuffer buffer(&bytes);
            buffer.open(QIODevice::WriteOnly);
            pixmap.save(&buffer, "PNG");
            album["cover"] = QString::fromLatin1(bytes.toBase64());
        }

        
        for (const QJsonObject &existing : albums) {
            if (existing["title"].toString() == title) {
                if (existing.contains("tracks")) {
                    album["tracks"] = existing["tracks"];
                }
                break;
            }
        }

        albumArray.append(album);
    }

    QJsonDocument doc(albumArray);
    QFile file("albums.json");
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
    } else {
        QMessageBox::warning(this, "Ошибка", "Не удалось сохранить файл!");
    }
}


void AlbumsWindow::loadData()
{
    model->removeRows(0, model->rowCount());

    QFile file("albums.json");
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    QJsonArray albumArray = doc.array();

    albums.clear();  

    for (const QJsonValue &value : albumArray)
    {
        QJsonObject obj = value.toObject();

        
        albums.append(obj);

        
        QPixmap pixmap;
        if (obj.contains("cover")) {
            QByteArray imageData = QByteArray::fromBase64(obj["cover"].toString().toLatin1());
            pixmap.loadFromData(imageData, "PNG");
        }

        
        QStandardItem *itemCover = new QStandardItem;
        itemCover->setEditable(false);
        itemCover->setText("");
        itemCover->setTextAlignment(Qt::AlignCenter);
        itemCover->setData(pixmap.scaled(100, 100, Qt::IgnoreAspectRatio, Qt::SmoothTransformation), Qt::DecorationRole);

        QStandardItem *itemTitle = new QStandardItem(obj["title"].toString());
        QStandardItem *itemArtist = new QStandardItem(obj["artist"].toString());
        QStandardItem *itemGenre = new QStandardItem(obj["genre"].toString());
        QStandardItem *itemYear = new QStandardItem(obj["year"].toString());

        for (auto *item : {itemTitle, itemArtist, itemGenre, itemYear}) {
            item->setTextAlignment(Qt::AlignCenter);
        }

        QList<QStandardItem*> row;
        row << itemCover << itemTitle << itemArtist << itemGenre << itemYear;
        model->appendRow(row);
    }
}



void AlbumsWindow::onAlbumDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    int row = index.row();
    QJsonObject albumObj = albums[row];  

    AlbumTracksWindow *tracksWindow = new AlbumTracksWindow(albumObj);
    connect(tracksWindow, &AlbumTracksWindow::albumUpdated, this, &AlbumsWindow::onAlbumUpdated);



    tracksWindow->setAttribute(Qt::WA_DeleteOnClose);
    tracksWindow->show();

}

void AlbumsWindow::openSelectedAlbum() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) return;

    int row = proxyModel->mapToSource(index).row();
    if (row < 0 || row >= albums.size()) return;

    QJsonObject selectedAlbum = albums[row];
    currentAlbum = selectedAlbum;

    AlbumTracksWindow *window = new AlbumTracksWindow(selectedAlbum, this);
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->show();
}

void AlbumsWindow::onOpenAlbumClicked() {
    QModelIndex index = ui->tableView->currentIndex();
    if (!index.isValid()) {
        QMessageBox::warning(this, "Ошибка", "Выберите альбом для открытия.");
        return;
    }

    int sourceRow = proxyModel->mapToSource(index).row();
    if (sourceRow < 0 || sourceRow >= model->rowCount())
        return;

    QString selectedTitle = model->item(sourceRow, 1)->text();
    QJsonObject selectedAlbum;

    for (const QJsonObject &a : albums) {
        if (a["title"].toString() == selectedTitle) {
            selectedAlbum = a;
            break;
        }
    }

    if (selectedAlbum.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Альбом не найден.");
        return;
    }

    currentAlbum = selectedAlbum;  

    AlbumTracksWindow *window = new AlbumTracksWindow(selectedAlbum, this);
    window->setAttribute(Qt::WA_DeleteOnClose);

    window->setWindowModality(Qt::ApplicationModal);
    window->show();

    connect(window, &AlbumTracksWindow::albumUpdated, this, [=](const QJsonObject &updatedAlbum) {
        bool updated = false;
        for (int i = 0; i < albums.size(); ++i) {
            if (albums[i]["title"].toString() == updatedAlbum["title"].toString()) {
                albums[i] = updatedAlbum;
                updated = true;
                break;
            }
        }
        if (!updated) {
            albums.append(updatedAlbum);
        }
        saveData();   
        loadData();   
    });



}




void AlbumsWindow::setAlbums(const QList<QJsonObject> &list) {
    albums = list;
    loadData();  
}
void AlbumsWindow::playTrackFromAlbum(int row)
{
    if (currentAlbum.isEmpty()) return;

    QJsonArray tracks = currentAlbum["tracks"].toArray();
    if (row < 0 || row >= tracks.size()) return;

    QJsonObject t = tracks[row].toObject();

    QString filePath = t["filePath"].toString();
    if (filePath.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Файл трека отсутствует.");
        return;
    }

    Track track(
        t["title"].toString(),
        t["duration"].toString(),
        filePath,
        t["artist"].toString(),
        t["genre"].toString(),
        t["year"].toString()
        );

    static PlayerWindow *player = nullptr;
    if (!player)
        player = new PlayerWindow();  

    player->setWindowFlag(Qt::Window); 
    player->setTrack(track);
    player->show();
    player->raise();
    player->activateWindow();

}
void AlbumsWindow::onAlbumUpdated(const QJsonObject &updatedAlbum)
{
    
    for (int i = 0; i < albums.size(); ++i) {
        if (albums[i]["title"].toString() == updatedAlbum["title"].toString()) {
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

    
    if (model) {
        model->clear();
        model->setColumnCount(3);
        model->setHeaderData(0, Qt::Horizontal, "Альбом");
        model->setHeaderData(1, Qt::Horizontal, "Исполнитель");
        model->setHeaderData(2, Qt::Horizontal, "Жанр");

        for (const QJsonObject &album : albums) {
            QList<QStandardItem*> row;
            row << new QStandardItem(album["title"].toString());
            row << new QStandardItem(album["artist"].toString());
            row << new QStandardItem(album["genre"].toString());
            model->appendRow(row);
        }
    }
}

void AlbumsWindow::refreshUI()
{
    model->clear();

    
    model->setColumnCount(3);
    model->setHeaderData(0, Qt::Horizontal, "Альбом");
    model->setHeaderData(1, Qt::Horizontal, "Исполнитель");
    model->setHeaderData(2, Qt::Horizontal, "Жанр");

    for (const QJsonObject &album : albums) {
        QList<QStandardItem*> row;
        row << new QStandardItem(album["title"].toString());
        row << new QStandardItem(album["artist"].toString());
        row << new QStandardItem(album["genre"].toString());
        model->appendRow(row);
    }
}

