/****************************************************************************
** Meta object code from reading C++ file 'GLWindow.h'
**
** Created: Wed Feb 1 18:36:05 2012
**      by: The Qt Meta Object Compiler version 62 (Qt 4.7.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../GLWindow.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'GLWindow.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.7.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_GLWindow[] = {

 // content:
       5,       // revision
       0,       // classname
       0,    0, // classinfo
      14,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x08,
      21,    9,    9,    9, 0x08,
      32,    9,    9,    9, 0x08,
      50,    9,    9,    9, 0x08,
      64,    9,    9,    9, 0x08,
      85,    9,    9,    9, 0x08,
     102,    9,    9,    9, 0x08,
     119,    9,    9,    9, 0x08,
     143,    9,    9,    9, 0x08,
     157,    9,    9,    9, 0x08,
     172,    9,    9,    9, 0x08,
     190,    9,    9,    9, 0x08,
     202,    9,    9,    9, 0x08,
     218,    9,    9,    9, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_GLWindow[] = {
    "GLWindow\0\0openFile()\0addFront()\0"
    "removeLastFront()\0toggleTimer()\0"
    "toggleRenderFronts()\0increaseGoalNV()\0"
    "decreaseGoalNV()\0exportPoseProjections()\0"
    "saveGLFrame()\0saveSVGFrame()\0"
    "toggleRecording()\0updateSVG()\0"
    "saveAnimation()\0toggleRidgeMode()\0"
};

const QMetaObject GLWindow::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_GLWindow,
      qt_meta_data_GLWindow, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &GLWindow::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *GLWindow::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *GLWindow::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_GLWindow))
        return static_cast<void*>(const_cast< GLWindow*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int GLWindow::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: openFile(); break;
        case 1: addFront(); break;
        case 2: removeLastFront(); break;
        case 3: toggleTimer(); break;
        case 4: toggleRenderFronts(); break;
        case 5: increaseGoalNV(); break;
        case 6: decreaseGoalNV(); break;
        case 7: exportPoseProjections(); break;
        case 8: saveGLFrame(); break;
        case 9: saveSVGFrame(); break;
        case 10: toggleRecording(); break;
        case 11: updateSVG(); break;
        case 12: saveAnimation(); break;
        case 13: toggleRidgeMode(); break;
        default: ;
        }
        _id -= 14;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
