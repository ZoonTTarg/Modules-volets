# Modules-volets
Modules implantés dans les interrupteurs des volets.
Un interrupteur mural doit toujours permettre d'activer le volet manuellement. Il est donc branché directement à l'arduino (moyennant quelques condensateurs pour éviter les rebonds).
C'est l'arduino qui se charge d'activer deux relais permettant la mise sous tension du moteur du volet, et le choix de la polarité. Reste à faire: émettre vers la box en cas d'activation manuelle pour mettre à jour la BDD avec le nouvel état du store.
Les modules servent aussi de répéteurs pour les autres installés dans la maison. Ce fonctionnement sera certainement supprimé par l'installation de modules RF HC-12 SI4463, qui ont une portée bien supérieure.

Réalisé:
Cablage de l'interrupteur et codage correspondant. L'activation des relais se fait correctement.
Réception des messages de la box ok, mais révision du mode de communication en cours: le module n'est pour l'instant plus fonctionnel.
