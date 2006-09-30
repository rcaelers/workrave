env = Environment()
env.Tool('qt')
env.Tool('mingw')
env.Replace(WR_ARCH='win32')
env.Append(CPPDEFINES={'HAVE_QT':1})
env.Append(CPPPATH=['.',
                    '#common/include',
                    '#common/include/$WR_ARCH',
                    '$QTDIR/include/QtGui',
                    '$QTDIR/include/QtCore'])
env.Replace(LIBS=['QtGui4','QtCore4'])
env.Append(LIBPATH=['$QTDIR/lib'])

env.SConscript(['backend/SConscript',
                'common/SConscript'], exports='env')

