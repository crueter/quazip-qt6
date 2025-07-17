#ifndef QUAZIP_QT_COMPAT_H
#define QUAZIP_QT_COMPAT_H

/*
 * For some reason, Qt 5.14 and 5.15 introduced a whole mess of seemingly random
 * moves and deprecations. To avoid populating code with #ifs,
 * we handle this stuff here, as well as some other compatibility issues.
 *
 * Some includes are repeated just in case we want to split this file later.
 */

#include <QtCore/Qt>
#include <QtCore/QtGlobal>

// Legacy encodings are still everywhere, but the Qt team decided we
// don't need them anymore and moved them out of Core in Qt 6.
#include <QtCore5Compat/QTextCodec>

// QSaveFile terribly breaks the is-a idiom (Liskov substitution principle):
// QSaveFile is-a QIODevice, but it makes close() private and aborts
// if you call it through the base class. Hence this ugly hack:
#include <QtCore/QSaveFile>
inline bool quazip_close(QIODevice *device) {
    QSaveFile *file = qobject_cast<QSaveFile*>(device);
    if (file != nullptr) {
        // We have to call the ugly commit() instead:
        return file->commit();
    }
    device->close();
    return true;
}

using Qt::SkipEmptyParts;

// and yet another... (why didn't they just make qSort delegate to std::sort?)
#include <QtCore/QList>
#include <algorithm>
template<typename T, typename C>
inline void quazip_sort(T begin, T end, C comparator) {
    std::sort(begin, end, comparator);
}

// this is a stupid rename...
#include <QtCore/QDateTime>
#include <QtCore/QFileInfo>
inline QDateTime quazip_ctime(const QFileInfo &fi) {
    return fi.birthTime();
}

// this is just a slightly better alternative
#include <QtCore/QFileInfo>
inline bool quazip_is_symlink(const QFileInfo &fi) {
    return fi.isSymbolicLink();
}

// I'm not even sure what this one is, but nevertheless
#include <QtCore/QFileInfo>
inline QString quazip_symlink_target(const QFileInfo &fi) {
    return fi.symLinkTarget();
}

// deprecation
#if QT_VERSION >= QT_VERSION_CHECK(6, 8, 0)
#include <QtCore/QTimeZone>
inline QDateTime quazip_since_epoch() {
    return QDateTime(QDate(1970, 1, 1), QTime(0, 0), QTimeZone::utc());
}

inline QDateTime quazip_since_epoch_ntfs() {
    return QDateTime(QDate(1601, 1, 1), QTime(0, 0), QTimeZone::utc());
}
#else
inline QDateTime quazip_since_epoch() {
    return QDateTime(QDate(1970, 1, 1), QTime(0, 0), Qt::UTC);
}

inline QDateTime quazip_since_epoch_ntfs() {
    return QDateTime(QDate(1601, 1, 1), QTime(0, 0), Qt::UTC);
}
#endif

// this is not a deprecation but an improvement, for a change
#include <QtCore/QDateTime>
inline quint64 quazip_ntfs_ticks(const QDateTime &time, int fineTicks) {
    QDateTime base = quazip_since_epoch_ntfs();
    return base.msecsTo(time) * 10000 + fineTicks;
}

// yet another improvement...
#include <QtCore/QDateTime>
inline qint64 quazip_to_time64_t(const QDateTime &time) {
    return time.toSecsSinceEpoch();
}

#include <QtCore/QTextStream>
const auto quazip_endl = Qt::endl;

#endif // QUAZIP_QT_COMPAT_H

