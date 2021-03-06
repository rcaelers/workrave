struct language_t
{
   const char *code;
   const char *lang;
};

static language_t languages []= 
{
{ "aa", "Afar" },
{ "ab", "Abkhazian" },
{ "ace", "Achinese" },
{ "ach", "Acoli" },
{ "ada", "Adangme" },
{ "ady", "Adyghe; Adygei" },
{ "ae", "Avestan" },
{ "af", "Afrikaans" },
{ "afa", "Afro-Asiatic languages" },
{ "afh", "Afrihili" },
{ "ain", "Ainu" },
{ "ak", "Akan" },
{ "akk", "Akkadian" },
{ "ale", "Aleut" },
{ "alg", "Algonquian languages" },
{ "alt", "Southern Altai" },
{ "am", "Amharic" },
{ "an", "Aragonese" },
{ "ang", "English, Old (ca. 450-1100)" },
{ "anp", "Angika" },
{ "apa", "Apache languages" },
{ "ar", "Arabic" },
{ "arc", "Official Aramaic (700-300 BCE); Imperial Aramaic (700-300 BCE)" },
{ "arn", "Mapudungun; Mapuche" },
{ "arp", "Arapaho" },
{ "art", "Artificial languages" },
{ "arw", "Arawak" },
{ "as", "Assamese" },
{ "ast", "Asturian; Bable; Leonese; Asturleonese" },
{ "ath", "Athapascan languages" },
{ "aus", "Australian languages" },
{ "av", "Avaric" },
{ "awa", "Awadhi" },
{ "ay", "Aymara" },
{ "az", "Azerbaijani" },
{ "ba", "Bashkir" },
{ "bad", "Banda languages" },
{ "bai", "Bamileke languages" },
{ "bal", "Baluchi" },
{ "ban", "Balinese" },
{ "bas", "Basa" },
{ "bat", "Baltic languages" },
{ "be", "Belarusian" },
{ "bej", "Beja; Bedawiyet" },
{ "bem", "Bemba" },
{ "ber", "Berber languages" },
{ "bg", "Bulgarian" },
{ "bh", "Bihari languages" },
{ "bho", "Bhojpuri" },
{ "bi", "Bislama" },
{ "bik", "Bikol" },
{ "bin", "Bini; Edo" },
{ "bla", "Siksika" },
{ "bm", "Bambara" },
{ "bn", "Bengali" },
{ "bnt", "Bantu languages" },
{ "bo", "Tibetan" },
{ "br", "Breton" },
{ "bra", "Braj" },
{ "bs", "Bosnian" },
{ "btk", "Batak languages" },
{ "bua", "Buriat" },
{ "bug", "Buginese" },
{ "byn", "Blin; Bilin" },
{ "ca", "Catalan; Valencian" },
{ "cad", "Caddo" },
{ "cai", "Central American Indian languages" },
{ "car", "Galibi Carib" },
{ "cau", "Caucasian languages" },
{ "ce", "Chechen" },
{ "ceb", "Cebuano" },
{ "cel", "Celtic languages" },
{ "ch", "Chamorro" },
{ "chb", "Chibcha" },
{ "chg", "Chagatai" },
{ "chk", "Chuukese" },
{ "chm", "Mari" },
{ "chn", "Chinook jargon" },
{ "cho", "Choctaw" },
{ "chp", "Chipewyan; Dene Suline" },
{ "chr", "Cherokee" },
{ "chy", "Cheyenne" },
{ "cmc", "Chamic languages" },
{ "co", "Corsican" },
{ "cop", "Coptic" },
{ "cpe", "Creoles and pidgins, English based" },
{ "cpf", "Creoles and pidgins, French-based" },
{ "cpp", "Creoles and pidgins, Portuguese-based" },
{ "cr", "Cree" },
{ "crh", "Crimean Tatar; Crimean Turkish" },
{ "crp", "Creoles and pidgins" },
{ "cs", "Czech" },
{ "csb", "Kashubian" },
{ "cu", "Church Slavic; Old Slavonic; Church Slavonic; Old Bulgarian; Old Church Slavonic" },
{ "cus", "Cushitic languages" },
{ "cv", "Chuvash" },
{ "cy", "Welsh" },
{ "da", "Danish" },
{ "dak", "Dakota" },
{ "dar", "Dargwa" },
{ "day", "Land Dayak languages" },
{ "de", "German" },
{ "del", "Delaware" },
{ "den", "Slave (Athapascan)" },
{ "dgr", "Dogrib" },
{ "din", "Dinka" },
{ "doi", "Dogri" },
{ "dra", "Dravidian languages" },
{ "dsb", "Lower Sorbian" },
{ "dua", "Duala" },
{ "dum", "Dutch, Middle (ca. 1050-1350)" },
{ "dv", "Divehi; Dhivehi; Maldivian" },
{ "dyu", "Dyula" },
{ "dz", "Dzongkha" },
{ "ee", "Ewe" },
{ "efi", "Efik" },
{ "egy", "Egyptian (Ancient)" },
{ "eka", "Ekajuk" },
{ "el", "Greek, Modern (1453-)" },
{ "elx", "Elamite" },
{ "en", "English" },
{ "enm", "English, Middle (1100-1500)" },
{ "eo", "Esperanto" },
{ "es", "Spanish; Castilian" },
{ "et", "Estonian" },
{ "eu", "Basque" },
{ "ewo", "Ewondo" },
{ "fa", "Persian" },
{ "fan", "Fang" },
{ "fat", "Fanti" },
{ "ff", "Fulah" },
{ "fi", "Finnish" },
{ "fil", "Filipino; Pilipino" },
{ "fiu", "Finno-Ugrian languages" },
{ "fj", "Fijian" },
{ "fo", "Faroese" },
{ "fon", "Fon" },
{ "fr", "French" },
{ "frm", "French, Middle (ca. 1400-1600)" },
{ "fro", "French, Old (842-ca. 1400)" },
{ "frr", "Northern Frisian" },
{ "frs", "Eastern Frisian" },
{ "fur", "Friulian" },
{ "fy", "Western Frisian" },
{ "ga", "Irish" },
{ "gaa", "Ga" },
{ "gay", "Gayo" },
{ "gba", "Gbaya" },
{ "gd", "Gaelic; Scottish Gaelic" },
{ "gem", "Germanic languages" },
{ "gez", "Geez" },
{ "gil", "Gilbertese" },
{ "gl", "Galician" },
{ "gmh", "German, Middle High (ca. 1050-1500)" },
{ "gn", "Guarani" },
{ "goh", "German, Old High (ca. 750-1050)" },
{ "gon", "Gondi" },
{ "gor", "Gorontalo" },
{ "got", "Gothic" },
{ "grb", "Grebo" },
{ "grc", "Greek, Ancient (to 1453)" },
{ "gsw", "Swiss German; Alemannic; Alsatian" },
{ "gu", "Gujarati" },
{ "gv", "Manx" },
{ "gwi", "Gwich'in" },
{ "ha", "Hausa" },
{ "hai", "Haida" },
{ "haw", "Hawaiian" },
{ "he", "Hebrew" },
{ "hi", "Hindi" },
{ "hil", "Hiligaynon" },
{ "him", "Himachali languages; Western Pahari languages" },
{ "hit", "Hittite" },
{ "hmn", "Hmong; Mong" },
{ "ho", "Hiri Motu" },
{ "hr", "Croatian" },
{ "hsb", "Upper Sorbian" },
{ "ht", "Haitian; Haitian Creole" },
{ "hu", "Hungarian" },
{ "hup", "Hupa" },
{ "hy", "Armenian" },
{ "hz", "Herero" },
{ "ia", "Interlingua (International Auxiliary Language Association)" },
{ "iba", "Iban" },
{ "id", "Indonesian" },
{ "ie", "Interlingue; Occidental" },
{ "ig", "Igbo" },
{ "ii", "Sichuan Yi; Nuosu" },
{ "ijo", "Ijo languages" },
{ "ik", "Inupiaq" },
{ "ilo", "Iloko" },
{ "inc", "Indic languages" },
{ "ine", "Indo-European languages" },
{ "inh", "Ingush" },
{ "io", "Ido" },
{ "ira", "Iranian languages" },
{ "iro", "Iroquoian languages" },
{ "is", "Icelandic" },
{ "it", "Italian" },
{ "iu", "Inuktitut" },
{ "ja", "Japanese" },
{ "jbo", "Lojban" },
{ "jpr", "Judeo-Persian" },
{ "jrb", "Judeo-Arabic" },
{ "jv", "Javanese" },
{ "ka", "Georgian" },
{ "kaa", "Kara-Kalpak" },
{ "kab", "Kabyle" },
{ "kac", "Kachin; Jingpho" },
{ "kam", "Kamba" },
{ "kar", "Karen languages" },
{ "kaw", "Kawi" },
{ "kbd", "Kabardian" },
{ "kg", "Kongo" },
{ "kha", "Khasi" },
{ "khi", "Khoisan languages" },
{ "kho", "Khotanese;Sakan" },
{ "ki", "Kikuyu; Gikuyu" },
{ "kj", "Kuanyama; Kwanyama" },
{ "kk", "Kazakh" },
{ "kl", "Kalaallisut; Greenlandic" },
{ "km", "Central Khmer" },
{ "kmb", "Kimbundu" },
{ "kn", "Kannada" },
{ "ko", "Korean" },
{ "kok", "Konkani" },
{ "kos", "Kosraean" },
{ "kpe", "Kpelle" },
{ "kr", "Kanuri" },
{ "krc", "Karachay-Balkar" },
{ "krl", "Karelian" },
{ "kro", "Kru languages" },
{ "kru", "Kurukh" },
{ "ks", "Kashmiri" },
{ "ku", "Kurdish" },
{ "kum", "Kumyk" },
{ "kut", "Kutenai" },
{ "kv", "Komi" },
{ "kw", "Cornish" },
{ "ky", "Kirghiz; Kyrgyz" },
{ "la", "Latin" },
{ "lad", "Ladino" },
{ "lah", "Lahnda" },
{ "lam", "Lamba" },
{ "lb", "Luxembourgish; Letzeburgesch" },
{ "lez", "Lezghian" },
{ "lg", "Ganda" },
{ "li", "Limburgan; Limburger; Limburgish" },
{ "ln", "Lingala" },
{ "lo", "Lao" },
{ "lol", "Mongo" },
{ "loz", "Lozi" },
{ "lt", "Lithuanian" },
{ "lu", "Luba-Katanga" },
{ "lua", "Luba-Lulua" },
{ "lui", "Luiseno" },
{ "lun", "Lunda" },
{ "luo", "Luo (Kenya and Tanzania)" },
{ "lus", "Lushai" },
{ "lv", "Latvian" },
{ "mad", "Madurese" },
{ "mag", "Magahi" },
{ "mai", "Maithili" },
{ "mak", "Makasar" },
{ "man", "Mandingo" },
{ "map", "Austronesian languages" },
{ "mas", "Masai" },
{ "mdf", "Moksha" },
{ "mdr", "Mandar" },
{ "men", "Mende" },
{ "mg", "Malagasy" },
{ "mga", "Irish, Middle (900-1200)" },
{ "mh", "Marshallese" },
{ "mi", "Maori" },
{ "mic", "Mi'kmaq; Micmac" },
{ "min", "Minangkabau" },
{ "mis", "Uncoded languages" },
{ "mk", "Macedonian" },
{ "mkh", "Mon-Khmer languages" },
{ "ml", "Malayalam" },
{ "mn", "Mongolian" },
{ "mnc", "Manchu" },
{ "mni", "Manipuri" },
{ "mno", "Manobo languages" },
{ "mo", "Moldavian; Moldovan" },
{ "moh", "Mohawk" },
{ "mos", "Mossi" },
{ "mr", "Marathi" },
{ "ms", "Malay" },
{ "mt", "Maltese" },
{ "mul", "Multiple languages" },
{ "mun", "Munda languages" },
{ "mus", "Creek" },
{ "mwl", "Mirandese" },
{ "mwr", "Marwari" },
{ "my", "Burmese" },
{ "myn", "Mayan languages" },
{ "myv", "Erzya" },
{ "na", "Nauru" },
{ "nah", "Nahuatl languages" },
{ "nai", "North American Indian languages" },
{ "nap", "Neapolitan" },
{ "nb", "Bokmål, Norwegian; Norwegian Bokmål" },
{ "nd", "Ndebele, North; North Ndebele" },
{ "nds", "Low German; Low Saxon; German, Low; Saxon, Low" },
{ "ne", "Nepali" },
{ "new", "Nepal Bhasa; Newari" },
{ "ng", "Ndonga" },
{ "nia", "Nias" },
{ "nic", "Niger-Kordofanian languages" },
{ "niu", "Niuean" },
{ "nl", "Dutch; Flemish" },
{ "nn", "Norwegian Nynorsk; Nynorsk, Norwegian" },
{ "no", "Norwegian" },
{ "nog", "Nogai" },
{ "non", "Norse, Old" },
{ "nqo", "N'Ko" },
{ "nr", "Ndebele, South; South Ndebele" },
{ "nso", "Pedi; Sepedi; Northern Sotho" },
{ "nub", "Nubian languages" },
{ "nv", "Navajo; Navaho" },
{ "nwc", "Classical Newari; Old Newari; Classical Nepal Bhasa" },
{ "ny", "Chichewa; Chewa; Nyanja" },
{ "nym", "Nyamwezi" },
{ "nyn", "Nyankole" },
{ "nyo", "Nyoro" },
{ "nzi", "Nzima" },
{ "oc", "Occitan (post 1500)" },
{ "oj", "Ojibwa" },
{ "om", "Oromo" },
{ "or", "Oriya" },
{ "os", "Ossetian; Ossetic" },
{ "osa", "Osage" },
{ "ota", "Turkish, Ottoman (1500-1928)" },
{ "oto", "Otomian languages" },
{ "pa", "Panjabi; Punjabi" },
{ "paa", "Papuan languages" },
{ "pag", "Pangasinan" },
{ "pal", "Pahlavi" },
{ "pam", "Pampanga; Kapampangan" },
{ "pap", "Papiamento" },
{ "pau", "Palauan" },
{ "peo", "Persian, Old (ca. 600-400 B.C.)" },
{ "phi", "Philippine languages" },
{ "phn", "Phoenician" },
{ "pi", "Pali" },
{ "pl", "Polish" },
{ "pon", "Pohnpeian" },
{ "pra", "Prakrit languages" },
{ "pro", "Provençal, Old (to 1500); Occitan, Old (to 1500)" },
{ "ps", "Pushto; Pashto" },
{ "pt", "Portuguese" },
{ "qaa-qtz", "Reserved for local use" },
{ "qu", "Quechua" },
{ "raj", "Rajasthani" },
{ "rap", "Rapanui" },
{ "rar", "Rarotongan; Cook Islands Maori" },
{ "rm", "Romansh" },
{ "rn", "Rundi" },
{ "ro", "Romanian" },
{ "roa", "Romance languages" },
{ "rom", "Romany" },
{ "ru", "Russian" },
{ "rup", "Aromanian; Arumanian; Macedo-Romanian" },
{ "rw", "Kinyarwanda" },
{ "sa", "Sanskrit" },
{ "sad", "Sandawe" },
{ "sah", "Yakut" },
{ "sai", "South American Indian languages" },
{ "sal", "Salishan languages" },
{ "sam", "Samaritan Aramaic" },
{ "sas", "Sasak" },
{ "sat", "Santali" },
{ "sc", "Sardinian" },
{ "scn", "Sicilian" },
{ "sco", "Scots" },
{ "sd", "Sindhi" },
{ "se", "Northern Sami" },
{ "sel", "Selkup" },
{ "sem", "Semitic languages" },
{ "sg", "Sango" },
{ "sga", "Irish, Old (to 900)" },
{ "sgn", "Sign Languages" },
{ "shn", "Shan" },
{ "si", "Sinhala; Sinhalese" },
{ "sid", "Sidamo" },
{ "sio", "Siouan languages" },
{ "sit", "Sino-Tibetan languages" },
{ "sk", "Slovak" },
{ "sl", "Slovenian" },
{ "sla", "Slavic languages" },
{ "sm", "Samoan" },
{ "sma", "Southern Sami" },
{ "smi", "Sami languages" },
{ "smj", "Lule Sami" },
{ "smn", "Inari Sami" },
{ "sms", "Skolt Sami" },
{ "sn", "Shona" },
{ "snk", "Soninke" },
{ "so", "Somali" },
{ "sog", "Sogdian" },
{ "son", "Songhai languages" },
{ "sq", "Albanian" },
{ "sr", "Serbian" },
{ "srn", "Sranan Tongo" },
{ "srr", "Serer" },
{ "ss", "Swati" },
{ "ssa", "Nilo-Saharan languages" },
{ "st", "Sotho, Southern" },
{ "su", "Sundanese" },
{ "suk", "Sukuma" },
{ "sus", "Susu" },
{ "sux", "Sumerian" },
{ "sv", "Swedish" },
{ "sw", "Swahili" },
{ "syc", "Classical Syriac" },
{ "syr", "Syriac" },
{ "ta", "Tamil" },
{ "tai", "Tai languages" },
{ "te", "Telugu" },
{ "tem", "Timne" },
{ "ter", "Tereno" },
{ "tet", "Tetum" },
{ "tg", "Tajik" },
{ "th", "Thai" },
{ "ti", "Tigrinya" },
{ "tig", "Tigre" },
{ "tiv", "Tiv" },
{ "tk", "Turkmen" },
{ "tkl", "Tokelau" },
{ "tl", "Tagalog" },
{ "tlh", "Klingon; tlhIngan-Hol" },
{ "tli", "Tlingit" },
{ "tmh", "Tamashek" },
{ "tn", "Tswana" },
{ "to", "Tonga (Tonga Islands)" },
{ "tog", "Tonga (Nyasa)" },
{ "tpi", "Tok Pisin" },
{ "tr", "Turkish" },
{ "ts", "Tsonga" },
{ "tsi", "Tsimshian" },
{ "tt", "Tatar" },
{ "tum", "Tumbuka" },
{ "tup", "Tupi languages" },
{ "tut", "Altaic languages" },
{ "tvl", "Tuvalu" },
{ "tw", "Twi" },
{ "ty", "Tahitian" },
{ "tyv", "Tuvinian" },
{ "udm", "Udmurt" },
{ "ug", "Uighur; Uyghur" },
{ "uga", "Ugaritic" },
{ "uk", "Ukrainian" },
{ "umb", "Umbundu" },
{ "und", "Undetermined" },
{ "ur", "Urdu" },
{ "uz", "Uzbek" },
{ "vai", "Vai" },
{ "ve", "Venda" },
{ "vi", "Vietnamese" },
{ "vo", "Volapük" },
{ "vot", "Votic" },
{ "wa", "Walloon" },
{ "wak", "Wakashan languages" },
{ "wal", "Wolaitta; Wolaytta" },
{ "war", "Waray" },
{ "was", "Washo" },
{ "wen", "Sorbian languages" },
{ "wo", "Wolof" },
{ "xal", "Kalmyk; Oirat" },
{ "xh", "Xhosa" },
{ "yao", "Yao" },
{ "yap", "Yapese" },
{ "yi", "Yiddish" },
{ "yo", "Yoruba" },
{ "ypk", "Yupik languages" },
{ "za", "Zhuang; Chuang" },
{ "zap", "Zapotec" },
{ "zbl", "Blissymbols; Blissymbolics; Bliss" },
{ "zen", "Zenaga" },
{ "zgh", "Standard Moroccan Tamazight" },
{ "zh", "Chinese" },
{ "znd", "Zande languages" },
{ "zu", "Zulu" },
{ "zun", "Zuni" },
{ "zxx", "No linguistic content; Not applicable" },
{ "zza", "Zaza; Dimili; Dimli; Kirdki; Kirmanjki; Zazaki" },
};

struct country_t
{
   const char *code;
   const char *country;
};

static country_t countries []= 
{
{ "AD", "Andorra" },
{ "AE", "United Arab Emirates" },
{ "AF", "Afghanistan" },
{ "AG", "Antigua and Barbuda" },
{ "AI", "Anguilla" },
{ "AL", "Albania" },
{ "AM", "Armenia" },
{ "AO", "Angola" },
{ "AQ", "Antarctica" },
{ "AR", "Argentina" },
{ "AS", "American Samoa" },
{ "AT", "Austria" },
{ "AU", "Australia" },
{ "AW", "Aruba" },
{ "AX", "Åland Islands" },
{ "AZ", "Azerbaijan" },
{ "BA", "Bosnia and Herzegovina" },
{ "BB", "Barbados" },
{ "BD", "Bangladesh" },
{ "BE", "Belgium" },
{ "BF", "Burkina Faso" },
{ "BG", "Bulgaria" },
{ "BH", "Bahrain" },
{ "BI", "Burundi" },
{ "BJ", "Benin" },
{ "BL", "Saint Barthélemy" },
{ "BM", "Bermuda" },
{ "BN", "Brunei Darussalam" },
{ "BO", "Bolivia" },
{ "BQ", "Bonaire, Sint Eustatius and Saba" },
{ "BR", "Brazil" },
{ "BS", "Bahamas" },
{ "BT", "Bhutan" },
{ "BV", "Bouvet Island" },
{ "BW", "Botswana" },
{ "BY", "Belarus" },
{ "BZ", "Belize" },
{ "CA", "Canada" },
{ "CC", "Cocos (Keeling) Islands" },
{ "CD", "Congo, The Democratic Republic of the" },
{ "CF", "Central African Republic" },
{ "CG", "Congo" },
{ "CH", "Switzerland" },
{ "CI", "Côte d'Ivoire" },
{ "CK", "Cook Islands" },
{ "CL", "Chile" },
{ "CM", "Cameroon" },
{ "CN", "China" },
{ "CO", "Colombia" },
{ "CR", "Costa Rica" },
{ "CU", "Cuba" },
{ "CV", "Cape Verde" },
{ "CW", "Curaçao" },
{ "CX", "Christmas Island" },
{ "CY", "Cyprus" },
{ "CZ", "Czech Republic" },
{ "DE", "Germany" },
{ "DJ", "Djibouti" },
{ "DK", "Denmark" },
{ "DM", "Dominica" },
{ "DO", "Dominican Republic" },
{ "DZ", "Algeria" },
{ "EC", "Ecuador" },
{ "EE", "Estonia" },
{ "EG", "Egypt" },
{ "EH", "Western Sahara" },
{ "ER", "Eritrea" },
{ "ES", "Spain" },
{ "ET", "Ethiopia" },
{ "FI", "Finland" },
{ "FJ", "Fiji" },
{ "FK", "Falkland Islands (Malvinas)" },
{ "FM", "Micronesia, Federated States of" },
{ "FO", "Faroe Islands" },
{ "FR", "France" },
{ "GA", "Gabon" },
{ "GB", "United Kingdom" },
{ "GD", "Grenada" },
{ "GE", "Georgia" },
{ "GF", "French Guiana" },
{ "GG", "Guernsey" },
{ "GH", "Ghana" },
{ "GI", "Gibraltar" },
{ "GL", "Greenland" },
{ "GM", "Gambia" },
{ "GN", "Guinea" },
{ "GP", "Guadeloupe" },
{ "GQ", "Equatorial Guinea" },
{ "GR", "Greece" },
{ "GS", "South Georgia and the South Sandwich Islands" },
{ "GT", "Guatemala" },
{ "GU", "Guam" },
{ "GW", "Guinea-Bissau" },
{ "GY", "Guyana" },
{ "HK", "Hong Kong" },
{ "HM", "Heard Island and McDonald Islands" },
{ "HN", "Honduras" },
{ "HR", "Croatia" },
{ "HT", "Haiti" },
{ "HU", "Hungary" },
{ "ID", "Indonesia" },
{ "IE", "Ireland" },
{ "IL", "Israel" },
{ "IM", "Isle of Man" },
{ "IN", "India" },
{ "IO", "British Indian Ocean Territory" },
{ "IQ", "Iraq" },
{ "IR", "Iran, Islamic Republic of" },
{ "IS", "Iceland" },
{ "IT", "Italy" },
{ "JE", "Jersey" },
{ "JM", "Jamaica" },
{ "JO", "Jordan" },
{ "JP", "Japan" },
{ "KE", "Kenya" },
{ "KG", "Kyrgyzstan" },
{ "KH", "Cambodia" },
{ "KI", "Kiribati" },
{ "KM", "Comoros" },
{ "KN", "Saint Kitts and Nevis" },
{ "KP", "Korea, Democratic People's Republic of" },
{ "KR", "Korea, Republic of" },
{ "KW", "Kuwait" },
{ "KY", "Cayman Islands" },
{ "KZ", "Kazakhstan" },
{ "LA", "Lao People's Democratic Republic" },
{ "LB", "Lebanon" },
{ "LC", "Saint Lucia" },
{ "LI", "Liechtenstein" },
{ "LK", "Sri Lanka" },
{ "LR", "Liberia" },
{ "LS", "Lesotho" },
{ "LT", "Lithuania" },
{ "LU", "Luxembourg" },
{ "LV", "Latvia" },
{ "LY", "Libya" },
{ "MA", "Morocco" },
{ "MC", "Monaco" },
{ "MD", "Moldova" },
{ "ME", "Montenegro" },
{ "MF", "Saint Martin (French part)" },
{ "MG", "Madagascar" },
{ "MH", "Marshall Islands" },
{ "MK", "Macedonia, Republic of" },
{ "ML", "Mali" },
{ "MM", "Myanmar" },
{ "MN", "Mongolia" },
{ "MO", "Macao" },
{ "MP", "Northern Mariana Islands" },
{ "MQ", "Martinique" },
{ "MR", "Mauritania" },
{ "MS", "Montserrat" },
{ "MT", "Malta" },
{ "MU", "Mauritius" },
{ "MV", "Maldives" },
{ "MW", "Malawi" },
{ "MX", "Mexico" },
{ "MY", "Malaysia" },
{ "MZ", "Mozambique" },
{ "NA", "Namibia" },
{ "NC", "New Caledonia" },
{ "NE", "Niger" },
{ "NF", "Norfolk Island" },
{ "NG", "Nigeria" },
{ "NI", "Nicaragua" },
{ "NL", "Netherlands" },
{ "NO", "Norway" },
{ "NP", "Nepal" },
{ "NR", "Nauru" },
{ "NU", "Niue" },
{ "NZ", "New Zealand" },
{ "OM", "Oman" },
{ "PA", "Panama" },
{ "PE", "Peru" },
{ "PF", "French Polynesia" },
{ "PG", "Papua New Guinea" },
{ "PH", "Philippines" },
{ "PK", "Pakistan" },
{ "PL", "Poland" },
{ "PM", "Saint Pierre and Miquelon" },
{ "PN", "Pitcairn" },
{ "PR", "Puerto Rico" },
{ "PS", "Palestine, State of" },
{ "PT", "Portugal" },
{ "PW", "Palau" },
{ "PY", "Paraguay" },
{ "QA", "Qatar" },
{ "RE", "Réunion" },
{ "RO", "Romania" },
{ "RS", "Serbia" },
{ "RU", "Russian Federation" },
{ "RW", "Rwanda" },
{ "SA", "Saudi Arabia" },
{ "SB", "Solomon Islands" },
{ "SC", "Seychelles" },
{ "SD", "Sudan" },
{ "SE", "Sweden" },
{ "SG", "Singapore" },
{ "SH", "Saint Helena, Ascension and Tristan da Cunha" },
{ "SI", "Slovenia" },
{ "SJ", "Svalbard and Jan Mayen" },
{ "SK", "Slovakia" },
{ "SL", "Sierra Leone" },
{ "SM", "San Marino" },
{ "SN", "Senegal" },
{ "SO", "Somalia" },
{ "SR", "Suriname" },
{ "SS", "South Sudan" },
{ "ST", "Sao Tome and Principe" },
{ "SV", "El Salvador" },
{ "SX", "Sint Maarten (Dutch part)" },
{ "SY", "Syrian Arab Republic" },
{ "SZ", "Swaziland" },
{ "TC", "Turks and Caicos Islands" },
{ "TD", "Chad" },
{ "TF", "French Southern Territories" },
{ "TG", "Togo" },
{ "TH", "Thailand" },
{ "TJ", "Tajikistan" },
{ "TK", "Tokelau" },
{ "TL", "Timor-Leste" },
{ "TM", "Turkmenistan" },
{ "TN", "Tunisia" },
{ "TO", "Tonga" },
{ "TR", "Turkey" },
{ "TT", "Trinidad and Tobago" },
{ "TV", "Tuvalu" },
{ "TW", "Taiwan" },
{ "TZ", "Tanzania, United Republic of" },
{ "UA", "Ukraine" },
{ "UG", "Uganda" },
{ "UM", "United States Minor Outlying Islands" },
{ "US", "United States" },
{ "UY", "Uruguay" },
{ "UZ", "Uzbekistan" },
{ "VA", "Holy See (Vatican City State)" },
{ "VC", "Saint Vincent and the Grenadines" },
{ "VE", "Venezuela" },
{ "VG", "Virgin Islands, British" },
{ "VI", "Virgin Islands, U.S." },
{ "VN", "Viet Nam" },
{ "VU", "Vanuatu" },
{ "WF", "Wallis and Futuna" },
{ "WS", "Samoa" },
{ "YE", "Yemen" },
{ "YT", "Mayotte" },
{ "ZA", "South Africa" },
{ "ZM", "Zambia" },
{ "ZW", "Zimbabwe" },
};
