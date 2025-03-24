# area_merging

## Context: Ome2


## Description


## Configuration


## Utilisation


Paramètres :

* c [obligatoire] : chemin vers le fichier de configuration
* t [obligatoire] : nom de la table de travail
* s [obligatoire] : nom de la table source
* cc [obligatoire] : code pays simple


Exemples d'appels:

~~~
bin/au_merging --c path/to/config/epg_parmaters.ini --s prod.administrative_unit_2 --t work.administrative_unit_1_w --cc fr
~~~