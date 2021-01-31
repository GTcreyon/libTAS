/*
    Copyright 2015-2020 Clément Gallet <clement.gallet@ens-lyon.org>

    This file is part of libTAS.

    libTAS is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    libTAS is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with libTAS.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBTAS_MAINWINDOW_H_INCLUDED
#define LIBTAS_MAINWINDOW_H_INCLUDED

#include <QMainWindow>
#include <QActionGroup>
#include <QAction>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QLabel>
#include <QCheckBox>
#include <QGroupBox>
#include <QComboBox>
#include <forward_list>
#include <string>

#include "config.h"
#include "EncodeWindow.h"
#include "ExecutableWindow.h"
#include "InputWindow.h"
#include "ControllerTabWindow.h"
#include "GameInfoWindow.h"
#include "GameSpecificWindow.h"
#include "RamSearchWindow.h"
#include "RamWatchWindow.h"
#include "InputEditorWindow.h"
#include "OsdWindow.h"
#include "AnnotationsWindow.h"
#include "AutoSaveWindow.h"
#include "TimeTraceWindow.h"
#include "../GameLoop.h"
#include "../Context.h"

#include <thread>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(Context* c);
    ~MainWindow();

    std::thread game_thread;
    GameLoop *gameLoop;
    Context *context;

    EncodeWindow* encodeWindow;
    InputWindow* inputWindow;
    ExecutableWindow* executableWindow;
    ControllerTabWindow* controllerTabWindow;
    GameInfoWindow* gameInfoWindow;
    GameSpecificWindow* gameSpecificWindow;
    RamSearchWindow* ramSearchWindow;
    RamWatchWindow* ramWatchWindow;
    InputEditorWindow* inputEditorWindow;
    OsdWindow* osdWindow;
    AnnotationsWindow* annotationsWindow;
    AutoSaveWindow* autoSaveWindow;
    TimeTraceWindow* timeTraceWindow;

    QList<QWidget*> disabledWidgetsOnStart;
    QList<QAction*> disabledActionsOnStart;

    QAction *saveMovieAction;
    QAction *exportMovieAction;
    QAction *annotateMovieAction;

    QAction *autoRestartAction;
    QAction *variableFramerateAction;
    QActionGroup *movieEndGroup;
    QActionGroup *screenResGroup;

    QAction *renderSoftAction;
    QAction *renderPerfAction;
    QActionGroup *osdGroup;
    QAction *osdEncodeAction;

    QActionGroup *frequencyGroup;
    QActionGroup *bitDepthGroup;
    QActionGroup *channelGroup;
    QAction *muteAction;
    QAction *disableAction;

    QActionGroup *localeGroup;

    QActionGroup *timeMainGroup;
    QActionGroup *timeSecGroup;

    QAction *busyloopAction;
    QAction *preventSavefileAction;
    QAction *recycleThreadsAction;

    QActionGroup *savestateGroup;
    QAction *steamAction;
    QActionGroup *waitGroup;
    QActionGroup *asyncGroup;

    QActionGroup *debugStateGroup;
    QActionGroup *loggingOutputGroup;
    QActionGroup *loggingPrintGroup;
    QActionGroup *loggingExcludeGroup;

    QAction *configEncodeAction;
    QAction *toggleEncodeAction;

    QActionGroup *slowdownGroup;
    QActionGroup *fastforwardGroup;

    QAction *mouseAction;
    QAction *mouseModeAction;
    QAction *mouseWarpAction;
    QAction *mouseGameWarpAction;
    QActionGroup *joystickGroup;


    QComboBox *gamePath;
    QPushButton *browseGamePath;
    QLineEdit *cmdOptions;

    QLineEdit *moviePath;
    QPushButton *browseMoviePath;

    QLineEdit *authorField;

    QRadioButton *movieRecording;
    QRadioButton *moviePlayback;

    QSpinBox *fpsNumField;
    QSpinBox *fpsDenField;
    QLabel *fpsValues;

    QCheckBox *pauseCheck;
    QCheckBox *fastForwardCheck;

    QSpinBox *frameCount;
    QSpinBox *movieFrameCount;
    QSpinBox *rerecordCount;
    QLabel *currentLength;
    QLabel *movieLength;

    QSpinBox *initialTimeSec;
    QSpinBox *initialTimeNsec;

    QPushButton *launchButton;
    QPushButton *launchGdbButton;
    QPushButton *stopButton;

    QGroupBox *movieBox;

    QLabel *statusIcon;
    QLabel *statusSoft;
    QLabel *statusMute;


    /* Event filter function to prevent menu close when a checkable option is clicked */
    bool eventFilter(QObject *obj, QEvent *event);

private:
    /* Update the status bar */
    void updateStatusBar();

    /* Update movie parameters from movie file */
    void updateMovieParams();

    /* Update the list of recent gamepaths */
    void updateRecentGamepaths();

    /* Update UI elements that are often modified, triggered by a timer */
    void updateUIFrequent();

    /* Helper function to create a checkable action inside an action group */
    QAction *addActionCheckable(QActionGroup*& group, const QString& text, const QVariant &data, const QString& toolTip);
    QAction *addActionCheckable(QActionGroup*& group, const QString& text, const QVariant &data);

    /* Create the main window actions that will go in the menus */
    void createActions();

    /* Create the main window menus */
    void createMenus();

private slots:

    /* Update UI elements (mainly enable/disable) depending on
     * the game status (running/stopped), to prevent modifying values that
     * are not supposed to be modified when the game is running.
     */
    void updateStatus();

    /* Update UI elements when the shared config has changed (pause, fastforward,
     * encode, etc.
     */
    void updateSharedConfigChanged();

    /* Update UI elements when a config file is loaded */
    void updateUIFromConfig();

    /* Show an alert dialog containing alert_msg as text */
    void alertDialog(QString alert_msg);

    /* Show an alert dialog asking the user a question, and get the answer */
    void alertOffer(QString alert_msg, void* promise);

    /* Update framerate values */
    void updateFramerate();

    void slotLaunch();
    void slotStop();
    void slotBrowseGamePath();
    void slotGamePathChanged();
    void slotBrowseMoviePath();
    void slotMoviePathChanged();
    void slotSaveMovie();
    void slotExportMovie();
    void slotPause(bool checked);
    void slotFastForward(bool checked);
    void slotMovieEnable(bool checked);
    void slotMovieRecording();
    void slotToggleEncode();
    void slotMuteSound(bool checked);
    void slotDisableSound(bool checked);
    void slotRenderSoft(bool checked);
    void slotRenderPerf(bool checked);
    void slotSavestate();
    void slotDebugState();
    void slotLoggingPrint();
    void slotLoggingExclude();
    void slotSlowdown();
    void slotFastforwardMode();
    void slotScreenRes();
#ifdef LIBTAS_ENABLE_HUD
    void slotOsd();
    void slotOsdEncode(bool checked);
#endif
    void slotBusyLoop(bool checked);
    void slotPreventSavefile(bool checked);
    void slotMovieEnd();
    void slotPauseMovie();
    void slotRecycleThreads(bool checked);
    void slotSteam(bool checked);
    void slotAsyncEvents(bool checked);
    void slotCalibrateMouse();
    void slotAutoRestart(bool checked);
    void slotVariableFramerate(bool checked);
    void slotMouseMode(bool checked);
    void slotMouseWarp(bool checked);
    void slotMouseGameWarp(bool checked);
    void slotLuaExecute();
    void slotLuaReset();
};

#endif
