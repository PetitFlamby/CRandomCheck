# CRandomCheck

# TO DO

- Fonction permettant le calcul automatique de l'espace disponible pour les calculs
````C
void calculation() {
    printf("%d", system('free -h | mawk \'NR==2{$6}\' | cut -d \'G\' -f 1')) ;
}
````

- Calcul de l'Ecart-Type à partir de la mémoire partagée
````
float standardDeviation() {
    return 1.0 ;
}
```