# au_merging

## Context

Open Maps For Europe 2 est un projet qui a pour objectif de développer un nouveau processus de production dont la finalité est la construction d'un référentiel cartographique pan-européen à grande échelle (1:10 000).

L'élaboration de la chaîne de production a nécessité le développement d'un ensemble de composants logiciels qui constituent le projet [OME2](https://github.com/openmapsforeurope2/OME2).


## Description

Cette application est consacrée à la fusion des unités administratives. Les unité administratives sont représentés par des polygones. 

L'échelon administrif le plus grand est 1 et représente le territoire national. L'échelon le plus petit dépend du pays et de son découpage administratif. Dans le référentiel OME2 l'échelon le plus petit possible est 6.

Le plus souvent, les unités administratives d'échelon N sont une agrégation d'unités administrives d'échelon N+1. Dans ce cas les objets géographiques représentant les unités administratives d'un échelon donné peuvent être dérivés des objets d'un échelon supérieur par fusion.


## Fonctionnement

La table cible est la table de travail dans laquelle ont été extraite les surfaces administratives proches de la frontière.

La table source est une table des unités administratives d'un échelon supérieur à celui de la table cible (par soucis de rapidité de calcul on prend, si possible, l'échelon immédiatement supérieur).

L'algorithme détermine pour chaque objet de la table cible quels sont les objets (polygones) de la table source qui le constitue. Si l'un de ces objets sources à précédemment été mis à jour alors une nouvelle géométrie est calculée pour l'objet cible par fusion des objets sources qui le compose.
 

## Configuration

Les fichiers de configuration se trouvent dans le [dossier de configuration](https://github.com/openmapsforeurope2/au_merging/tree/main/config) et sont les suivants :
- epg_parameters.ini : regroupe des paramètres de base issus de la bibliothèque libepg qui constitue le socle de développement l'outil. Ce fichier est aussi le fichier chapeau qui pointe vers les autres fichiers de configurations.
- db_conf.ini : informations de connexion à la base de données.
- theme_parameters.ini : configuration des paramètres spécifiques à l'application.


## Utilisation

L'outil s'utilise en ligne de commande.

Paramètres :

* c [obligatoire] : chemin vers le fichier de configuration
* s [obligatoire] : suffix de la table de travail
* sl [obligatoire] : niveau administratif source
* tl [obligatoire] : niveau administratif cible
* argument libre [obligatoire] : code pays

<br>

Exemples d'appel:

~~~
bin/au_merging --c path/to/config/epg_parmaters.ini --tl 2 --sl 1 --s 20251113 fr
~~~