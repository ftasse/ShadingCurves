/****************************************************************************
** Meta object code from reading C++ file 'GLScene.h'
**
** Created: Wed Feb 6 16:44:16 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.3)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../Views/GLScene.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GLScene.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.3. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GLScene[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      13,    9,    8,    8, 0x0a,
      31,    9,    8,    8, 0x0a,
      56,   52,    8,    8, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_GLScene[] = {
    "GLScene\0\0cid\0addCurveItem(int)\0"
    "updateCurveItem(int)\0pid\0addPointItem(int)\0"
};

void GLScene::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        GLScene *_t = static_cast<GLScene *>(_o);
        switch (_id) {
        case 0: _t->addCurveItem((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->updateCurveItem((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->addPointItem((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData GLScene::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject GLScene::staticMetaObject = {
    { &QGraphicsScene::staticMetaObject, qt_meta_stringdata_GLScene,
      qt_meta_data_GLScene, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GLScene::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GLScene::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GLScene::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GLScene))
        return static_cast<void*>(const_cast< GLScene*>(this));
    return QGraphicsScene::qt_metacast(_clname);
}

int GLScene::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGraphicsScene::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
