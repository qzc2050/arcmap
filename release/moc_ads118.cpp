/****************************************************************************
** Meta object code from reading C++ file 'ads118.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.12.5)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../ads118.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ads118.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.12.5. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ADS118_t {
    QByteArrayData data[7];
    char stringdata0[79];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ADS118_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ADS118_t qt_meta_stringdata_ADS118 = {
    {
QT_MOC_LITERAL(0, 0, 6), // "ADS118"
QT_MOC_LITERAL(1, 7, 22), // "ads1115_config_channel"
QT_MOC_LITERAL(2, 30, 0), // ""
QT_MOC_LITERAL(3, 31, 7), // "uint8_t"
QT_MOC_LITERAL(4, 39, 7), // "channel"
QT_MOC_LITERAL(5, 47, 20), // "ads1115_read_ad_vale"
QT_MOC_LITERAL(6, 68, 10) // "doadstimer"

    },
    "ADS118\0ads1115_config_channel\0\0uint8_t\0"
    "channel\0ads1115_read_ad_vale\0doadstimer"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ADS118[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    1,   29,    2, 0x0a /* Public */,
       5,    1,   32,    2, 0x0a /* Public */,
       6,    0,   35,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void, 0x80000000 | 3,    4,
    QMetaType::Float, 0x80000000 | 3,    4,
    QMetaType::Void,

       0        // eod
};

void ADS118::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ADS118 *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->ads1115_config_channel((*reinterpret_cast< uint8_t(*)>(_a[1]))); break;
        case 1: { float _r = _t->ads1115_read_ad_vale((*reinterpret_cast< uint8_t(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< float*>(_a[0]) = std::move(_r); }  break;
        case 2: _t->doadstimer(); break;
        default: ;
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ADS118::staticMetaObject = { {
    &QObject::staticMetaObject,
    qt_meta_stringdata_ADS118.data,
    qt_meta_data_ADS118,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ADS118::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ADS118::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ADS118.stringdata0))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int ADS118::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
