#ifndef TRACK_H
#define TRACK_H

#include <QString>

struct Track {
    QString title;
    QString duration;
    QString filePath;
    QString artist;
    QString genre;
    QString year;

    Track() = default;
    Track(const QString &t, const QString &d, const QString &f, const QString &a, const QString &g, const QString &y)
        : title(t), duration(d), filePath(f), artist(a), genre(g), year(y) {}
};


#endif 
