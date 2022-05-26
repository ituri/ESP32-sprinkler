/*
    this is a file i use in all my esp32 mqtt projects to store the
    credentials and certificate to my mqtt server
    this is sample data - the original file ist stored 
    in my library folder so it can be included into all my projects
    and can be changed for all mqtt devices at once
    https://github.com/bugfx

    check out arduino-esp32 WiFiClientSecure fore more info:
    https://github.com/espressif/arduino-esp32/blob/master/libraries/WiFiClientSecure/examples/WiFiClientSecure/WiFiClientSecure.ino
*/
IPAddress mqttServer(192, 168, 1, 10);
const char mqttUser[] = "myMqttServerUserName";
const char mqttPass[] = "myMqttServerPasswd";
const char mqttPort[] = "8883";
const char mqttCaCert[] = \
"-----BEGIN CERTIFICATE-----\n" \
"yourMqttCertificate_OOOOewfeffwfesfrgergoienecwUGGUGUBdtguzgcQEL\n" \
"6jtr6jhrt6jt6jt6j67k67ktzj5e6j67k78tljktzjtzjtjt7jt7j7kt7ktktk7t\n" \
"Z2VuMRUwEwYDVQQKDAxIb21lIE5ldHdvcmsxDTALBgNVBAMMBHVzZXIwIBcNMjAx\n" \
"rtnhh589gh458iuvgIH45h8ojn43og845h98hj459nho4jn5bw4h59hnio5hno85\n" \
"ishghe4ggh4ghi834hibh5s4oihb9h5hb9h59hbi34shighi34hbsdgsdgdsgasf\n" \
"hiuvew4biugebvui4iuvbibigekrbgvkbui4bvuibsiu4vbisb4k5ugbvu4bb4ug\n" \
"vh4riebuerbibseriubiueruibierUIZeriubiuerbiuseruigukaerwbnvi4bnu\n" \
"ghv843hh89bh3s85jb5hj49hb8s4hbs48obhoshe4b5h48shbs4h5bs85b885hb5\n" \
"gj3498hb4hbh45hb78h854h87bh458bh5478hb84nZZ78h5n7845hb87h57845hb\n" \
"ih3gc9h348gf8n934gv89389ngnh549hgv95h4g5ch594ghvh54O8h5489gh54gg\n" \
"ut9g5n89gnz4589dgh49hgh548gh45hgv89hgb845hg458gh84hc5478ghc845hv\n" \
"gh7845ghv894Phjv54hj5ohgbv85h5ibhvi4h5biuhi45hbih45ibhi845hbh5i4\n" \
"5gh984hg89shghs4895ghi8hgbs45hv4bi5usvbi45iugbn4ui5bgui54hbgibni\n" \
"c85m489gh49hgv84hgv4895gvh9845hn45nv547hgv745hgv78c4m7455hd54vnb\n" \
"gih4iugbieh89bhe458hb4gh95h89h8943hgg98h5g8945hgh945hv4589gh945h\n" \
"gj8945hv89gn5f9w4ghd8w74hg98dh5498ghf945hnbv7845hncg845hfg94g454\n" \
"hfhrUWEFWEFG554E5HE5BJREHHvhehb945hgvh584cn3hg45hgv785hgnv45hgvJ\n" \
"J50aci3uhgci3hngv78eh478vghe78gh8e4gh8e4hg8vehighevo4ghjog4geg4W\n" \
"nsTciuehciuihhuGHIBKUUBkc48chgn845hg845hngv45v5ghm5g45hig45h54hh\n" \
"CA==\n" \
"-----END CERTIFICATE-----\n";
