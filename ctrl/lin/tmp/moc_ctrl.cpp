/****************************************************************************
** Meta object code from reading C++ file 'ctrl.h'
**
** Created: Fri Sep 30 10:56:47 2011
**      by: The Qt Meta Object Compiler version 62 (Qt 4.6.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../src/ctrl.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ctrl.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 62
#error "This file was generated using the moc from 4.6.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_ctrl[] = {

 // content:
       4,       // revision
       0,       // classname
       0,    0, // classinfo
       9,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       6,    5,    5,    5, 0x08,
      35,    5,    5,    5, 0x08,
      80,    5,    5,    5, 0x08,
     109,    5,    5,    5, 0x08,
     150,    5,    5,    5, 0x08,
     181,    5,    5,    5, 0x08,
     209,    5,    5,    5, 0x08,
     238,    5,    5,    5, 0x08,
     272,    5,    5,    5, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_ctrl[] = {
    "ctrl\0\0on_m_pClose_button_clicked()\0"
    "on_m_pVersion_combo_currentIndexChanged(int)\0"
    "on_m_pApply_button_clicked()\0"
    "on_m_pApp_combo_currentIndexChanged(int)\0"
    "on_m_pRestart_button_clicked()\0"
    "on_m_pStop_button_clicked()\0"
    "on_m_pStart_button_clicked()\0"
    "on_m_pChange_button_toggled(bool)\0"
    "update_state_slot()\0"
};

const QMetaObject ctrl::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_ctrl,
      qt_meta_data_ctrl, 0 }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &ctrl::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *ctrl::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *ctrl::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_ctrl))
        return static_cast<void*>(const_cast< ctrl*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int ctrl::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: on_m_pClose_button_clicked(); break;
        case 1: on_m_pVersion_combo_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: on_m_pApply_button_clicked(); break;
        case 3: on_m_pApp_combo_currentIndexChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 4: on_m_pRestart_button_clicked(); break;
        case 5: on_m_pStop_button_clicked(); break;
        case 6: on_m_pStart_button_clicked(); break;
        case 7: on_m_pChange_button_toggled((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 8: update_state_slot(); break;
        default: ;
        }
        _id -= 9;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
