#include "playlistswindow.h"
#include "ui_playlistswindow.h"
#include "playlisttrackswindow.h"
#include "playlisteditdialog.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QJsonDocument>
#include <QFile>
#include <QLabel>
#include <QPixmap>
#include <QDebug>

PlaylistsWindow::PlaylistsWindow(QWidget *parent, const QList<QJsonObject> &albumsList)
    : QWidget(parent), ui(new Ui::PlaylistsWindow), albums(albumsList)

{
    ui->setupUi(this);
    setWindowTitle("Плейлисты");
    setMinimumSize(600, 500); 
    setStyleSheet(R"(
    QWidget {
        background-color: #ffe6f0;
        color: #2e2e2e; 
        font-size: 14px;
    }

    QPushButton {
        background-color: #ff99cc;
        color: white;
        font-weight: bold;
        border-radius: 10px;
        height: 40px;
    }

    QPushButton:hover {
        background-color: #ff66b2;
    }

    QLabel {
        font-size: 18pt;
        font-weight: bold;
        color: #2e2e2e;
    }
)");



    loadPlaylists();
    refreshUI();
    connect(ui->btnEdit, &QPushButton::clicked, this, &PlaylistsWindow::editSelectedPlaylist);
    connect(ui->btnDelete, &QPushButton::clicked, this, &PlaylistsWindow::deleteSelectedPlaylist);



}


PlaylistsWindow::~PlaylistsWindow()
{
    delete ui;
}

void PlaylistsWindow::on_btnAddPlaylist_clicked()
{
    PlaylistEditDialog dlg(this);  
    if (dlg.exec() == QDialog::Accepted) {
        QJsonObject playlist = dlg.getData();
        playlists.append(playlist);
        savePlaylists();
        refreshUI();
    }
}


void PlaylistsWindow::loadPlaylists()
{
    QFile file("playlists.json");
    if (!file.open(QIODevice::ReadOnly))
        return;

    QByteArray data = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isArray())
        playlists = doc.array();
}
bool PlaylistsWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        int index = watched->property("playlistIndex").toInt();
        selectedPlaylistIndex = index;
        openPlaylistDetails(index);  
        return true;
    }
    return QWidget::eventFilter(watched, event);
}


void PlaylistsWindow::savePlaylists()
{
    QFile file("playlists.json");
    if (!file.open(QIODevice::WriteOnly))
        return;

    QJsonDocument doc(playlists);
    file.write(doc.toJson());
}

void PlaylistsWindow::refreshUI()
{
    QLayout *layout = ui->scrollAreaWidgetContents->layout();
    if (!layout) {
        layout = new QVBoxLayout;
        ui->scrollAreaWidgetContents->setLayout(layout);  
        layout->setAlignment(Qt::AlignTop);
    }

    
    QLayoutItem *child;
    while ((child = layout->takeAt(0)) != nullptr) {
        delete child->widget();
        delete child;
    }
    for (int i = 0; i < playlists.size(); ++i) {
        if (!playlists[i].isObject()) continue;

        QJsonObject obj = playlists[i].toObject();

        
        if (!obj.contains("tracks") || !obj["tracks"].isArray()) {
            obj["tracks"] = QJsonArray();
            playlists[i] = obj;
        }

        QString title = obj.contains("title") ? obj["title"].toString() : "Без названия";
        QString desc = obj.contains("description") ? obj["description"].toString() : "";
        QString coverPath = obj.contains("cover") ? obj["cover"].toString() : "";

        QWidget *card = new QWidget;
        card->setStyleSheet("background-color: #ff99cc; border-radius: 12px;");
        card->setFixedHeight(100);
        card->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        QHBoxLayout *cardLayout = new QHBoxLayout(card);
        cardLayout->setContentsMargins(10, 5, 10, 5);
        cardLayout->setSpacing(15);

        QLabel *img = new QLabel;
        QPixmap pix;
        if (!coverPath.isEmpty() && QFile::exists(coverPath)) {
            if (pix.load(coverPath)) {
                img->setPixmap(pix.scaled(80, 80, Qt::KeepAspectRatio, Qt::SmoothTransformation));
            } else {
                img->clear();  
            }
        } else {
            img->clear();  
        }


        img->setFixedSize(80, 80);

        QLabel *text = new QLabel;
        text->setTextFormat(Qt::RichText);
        text->setText(QString("<b style='font-size:16px;'>%1</b><br><span style='font-size:13px;'>%2</span>").arg(title, desc));
        text->setStyleSheet("color: white;");
        text->setAlignment(Qt::AlignVCenter);

        cardLayout->addWidget(img);
        cardLayout->addWidget(text);

        card->setProperty("playlistIndex", i);
        card->installEventFilter(this);

        layout->addWidget(card);
    }
}



void PlaylistsWindow::editSelectedPlaylist()
{
    if (selectedPlaylistIndex < 0 || selectedPlaylistIndex >= playlists.size()) return;

    QJsonObject playlist = playlists[selectedPlaylistIndex].toObject();
    PlaylistEditDialog dialog(this);
    dialog.setData(playlist);

    if (dialog.exec() == QDialog::Accepted) {
        playlists[selectedPlaylistIndex] = dialog.getData();
        savePlaylists();
        refreshUI();
    }
}


void PlaylistsWindow::deleteSelectedPlaylist()
{
    if (selectedPlaylistIndex < 0 || selectedPlaylistIndex >= playlists.size()) return;

    playlists.removeAt(selectedPlaylistIndex);
    savePlaylists();
    refreshUI();
}


void PlaylistsWindow::openPlaylistDetails(int index)
{
    if (index < 0 || index >= playlists.size()) return;

    QJsonObject playlist = playlists[index].toObject();

    auto *tracksWindow = new PlaylistTracksWindow(nullptr, albums); 
    tracksWindow->setPlaylist(playlist);
    tracksWindow->setAttribute(Qt::WA_DeleteOnClose);
    tracksWindow->show();

    this->hide();  

    
    connect(tracksWindow, &QObject::destroyed, this, [this]() {
        this->show();
    });
}


void PlaylistsWindow::setAlbums(const QList<QJsonObject> &albumsList)
{
    albums = albumsList;
}




