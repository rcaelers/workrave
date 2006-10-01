env = Environment()

env.Tool('qt')
# The qt tool is apparently qt3, links libqt. Qt4 is split into different libs
env.Replace(LIBS=[]) 

env.Tool('mingw')
env.Replace(WR_ARCH='win32')
env.Append(CPPDEFINES={'HAVE_QT':None,
                       'TIME_WITH_SYS_TIME':None})
env.Append(CPPPATH=['.',
                    '#common/include',
                    '#common/include/$WR_ARCH',
                    '$QTDIR/include/QtGui',
                    '$QTDIR/include/QtCore'])

env.SConscript(['backend/SConscript',
                'common/SConscript',
                'frontend/SConscript'], exports='env')

