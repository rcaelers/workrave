#!/bin/sh
# This is script that provides basic KDE interface for WorkRave.
# It does depend on kdialog and qdbus, which should be part of your
# KDE SC installation.
#
# Skrypt zapewnia podstawowy interfejs do WorkRave pod KDE.
# Do działania wymaga kdialog i qdbus, które powinny być częścią Twojej 
# instalacji KDE SC.
#
# Author: Mirosław „Minio” Zalewski <miniopl@gmail.com> http://minio.xt.pl
# Last modified: Fri, 03 Aug 2012 18:04:46 +0200
# License: None
##########################################################################

# basic l10n
# Currently only Polish and English (default) are supported.
# Patches for another languages are welcome.

case $LANG in
	pl_PL*)
		WIN_TITLE="WorkRave do KDE"
		OPEN_WINDOW="Okno główne"
		SET_NORMAL="Tryb normalny"
		SET_QUIET="Tryb cichy"
		SET_SUSPENDED="Tryb zawieszony"
		TOGGLE_READING="Przełącz tryb czytelniczy"
		PREFERENCES_WINDOW="Okno ustawień"
		STATISTICS_WINDOW="Okno statystyk"
		ENABLED="włączony"
		DISABLED="wyłączony"
		CURRENT_MODE="Aktualny tryb"
		NORMAL="Normalny"
		QUIET="Cichy"
		SUSPENDED="Zawieszony"
		MICRO_BREAK_TIME="Najbliższa mikroprzerwa za"
		REST_BREAK_TIME="Najbliższy odpoczynek za"
		READING_MODE="Tryb czytelniczy"
		ERROR="Błąd podczas wykonywania metody"
		FATAL_ERROR="Stało się coś bardzo złego. Ten komunikat błędu nie powinien \
			się nigdy wyświetlić."
		;;
	*)
		WIN_TITLE="WorkRave for KDE"
		OPEN_WINDOW="Open main window"
		SET_NORMAL="Set Normal mode"
		SET_QUIET="Set Quiet mode"
		SET_SUSPENDED="Set Suspended mode"
		TOGGLE_READING="Toggle Reading mode"
		PREFERENCES_WINDOW="Open preferences window"
		STATISTICS_WINDOW="Open statistics window"
		ENABLED="enabled"
		DISABLED="disabled"
		CURRENT_MODE="Current mode"
		NORMAL="Normal"
		QUIET="Quiet"
		SUSPENDED="Suspended"
		MICRO_BREAK_TIME="Time left to micro-break"
		REST_BREAK_TIME="Time left to rest break"
		READING_MODE="Reading mode"
		ERROR="Error while trying to access method"
		FATAL_ERROR="Something terrible had happened. You should have never seen \
			this message."
	;;
esac

PID_FILE="/tmp/workrave-kde.sh-is-running-${USER}"

clean_up() {
	rm -f "$PID_FILE" >/dev/null 2>&1
}

try() {
	if [ -z "$1" ]; then
		exit
	fi
	if ! ret=$(qdbus $SESSION_BUS $@) ; then
		kdialog --error "$(echo "$ERROR\n$@")"
		clean_up
		kill $$
	fi
	echo $ret
}

seconds_to_readable() {
	local secs="$1"
	h=$((secs/3600))
	m=$((secs%3600/60))
	s=$((secs%60))
	if [ "$h" -gt 0 ]; then
		echo -n "$h:"
	fi
	echo "$m:$s"
}

if [ -e "$PID_FILE" ]; then
	if which wmctrl >/dev/null 2>&1 ; then
		wmctrl -R "$WIN_TITLE"
	fi
	exit
fi
echo $$ > "$PID_FILE"

trap clean_up 0
trap "exit 2" 1 2 3 15

SESSION_BUS="org.workrave.Workrave"
METHOD_ROOT="/org/workrave/Workrave"
CORE_INTERFACE="$METHOD_ROOT/Core org.workrave.CoreInterface"
CONTROL_INTERFACE="$METHOD_ROOT/UI org.workrave.ControlInterface"
APPLET_INTERFACE="$METHOD_ROOT/UI org.workrave.AppletInterface"
CONFIG_INTERFACE="$METHOD_ROOT/Core org.workrave.ConfigInterface"

case "$(try "$CORE_INTERFACE.GetOperationMode")" in
	normal) CURRENT_OPERATION_MODE="$NORMAL" ;;
	quiet) CURRENT_OPERATION_MODE="$QUIET";;
	suspended) CURRENT_OPERATION_MODE="$SUSPENDED" ;;
esac

#TODO: This have to do until upstream provide interface for reading state of
# Reading mode (should be in 1.9.5 or 1.10, whichever comes first)
case "$(try "$CONFIG_INTERFACE.GetInt" "general/usage-mode" |tr -d [:blank:][:alpha:])" in
	0) READING_STATE="$DISABLED";;
	1) READING_STATE="$ENABLED";;
esac

microbreak_limit="$(try "$CONFIG_INTERFACE.GetInt" "timers/micro_pause/limit" |tr -d [:blank:][:alpha:])"
microbreak_current="$(try "$CORE_INTERFACE.GetTimerElapsed" "microbreak")"
next_microbreak=$((microbreak_limit - microbreak_current))
SELECT_ACTION="$MICRO_BREAK_TIME: $(seconds_to_readable "$next_microbreak")\n"

restbreak_limit="$(try "$CONFIG_INTERFACE.GetInt" "timers/rest_break/limit" |tr -d [:blank:][:alpha:])"
restbreak_current="$(try "$CORE_INTERFACE.GetTimerElapsed" "restbreak")"
next_restbreak=$((restbreak_limit - restbreak_current))
SELECT_ACTION="${SELECT_ACTION}$REST_BREAK_TIME: $(seconds_to_readable "$next_restbreak")\n"

SELECT_ACTION="${SELECT_ACTION}$CURRENT_MODE: $CURRENT_OPERATION_MODE\n"

SELECT_ACTION="${SELECT_ACTION}$READING_MODE: $READING_STATE"

action=$(kdialog --title "$WIN_TITLE" --icon workrave --menu "$(/bin/echo -e "$SELECT_ACTION")"\
	open "$OPEN_WINDOW" set_normal "$SET_NORMAL"	set_quiet "$SET_QUIET"\
	set_suspended "$SET_SUSPENDED" toggle_reading "$TOGGLE_READING"\
	prefs "$PREFERENCES_WINDOW" stats "$STATISTICS_WINDOW")

if [ -z "$action" ]; then
	exit
fi

case "$action" in
	open) try "$CONTROL_INTERFACE.OpenMain" ;;
	set_normal) try "$CORE_INTERFACE.SetOperationMode" "normal";;
	set_quiet) try "$CORE_INTERFACE.SetOperationMode" "quiet";;
	set_suspended) try "$CORE_INTERFACE.SetOperationMode" "suspended";;
	toggle_reading) try "$APPLET_INTERFACE.Command" "12";;
	prefs) try "$CONTROL_INTERFACE.Preferences";;
	stats) try "$CONTROL_INTERFACE.Statistics";;
	*) kdialog --error "$FATAL_ERROR";;
esac
clean_up
