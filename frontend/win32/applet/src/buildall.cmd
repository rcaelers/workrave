@ECHO OFF

CMD /V:ON /E:ON /C buildone.cmd DEBUG64
CMD /V:ON /E:ON /C buildone.cmd RELEASE64
CMD /V:ON /E:ON /C buildone.cmd DEBUG
CMD /V:ON /E:ON /C buildone.cmd RELEASE
