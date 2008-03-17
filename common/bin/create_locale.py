# LoclaeInfo.py (c) 2006 Canonical, released under the GPL
#
# a helper class to get locale info

import string
import re            
import subprocess
import gettext

from gettext import gettext as _
from xml.etree.ElementTree import ElementTree

languages = {}
countries = {}

et = ElementTree(file = "/usr/share/xml/iso-codes/iso_639.xml")
it = et.getiterator('iso_639_entry')
for elm in it:
    lang = elm.attrib["name"]
    if elm.attrib.has_key("iso_639_1_code"):
        code = elm.attrib["iso_639_1_code"]
    else:
        code = elm.attrib["iso_639_2T_code"]

    languages[code] = lang

et = ElementTree(file="/usr/share/xml/iso-codes/iso_3166.xml")
it = et.getiterator('iso_3166_entry')
for elm in it:
    if elm.attrib.has_key("common_name"):
        descr = elm.attrib["common_name"]
    else:
        descr = elm.attrib["name"]

    if elm.attrib.has_key("alpha_2_code"):
        code = elm.attrib["alpha_2_code"]
    else:
        code = elm.attrib["alpha_3_code"]

    countries[code] = descr

print "struct language_t"
print "{"
print "   const char *code;"
print "   const char *lang;"
print "} languages []= "
print "{"

keys = languages.keys()
keys.sort();

for k in keys:
    txt = "{ \"%s\", \"%s\" }," % (k, languages[k])
    print txt.encode('utf-8')

print "};"
print

print "struct country_t"
print "{"
print "   const char *code;"
print "   const char *country;"
print "} countries []= "
print "{"


keys = countries.keys()
keys.sort();

for k in keys:
    txt = "{ \"%s\", \"%s\" }," % (k, countries[k])
    print txt.encode('utf-8')

    
print "};"
