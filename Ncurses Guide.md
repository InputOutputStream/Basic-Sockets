# Guide des Fonctions ncurses et Bibliothèques Utilisées

## 📚 Bibliothèques Ajoutées

### `#include <pthread.h>`
**Rôle** : Gestion des threads POSIX pour la programmation multi-thread
- Permet au serveur de gérer plusieurs clients simultanément
- Chaque client connecté aura son propre thread

### `#include <time.h>`
**Rôle** : Gestion du temps et des timestamps
- Horodatage des messages
- Mesure des intervalles pour les mises à jour

---

## 🖥️ Fonctions ncurses Détaillées

### **Initialisation et Configuration**

#### `initscr()`
```c
initscr();
```
- **Rôle** : Initialise ncurses et le mode terminal
- **Action** : Passe le terminal en mode "screen" pour la gestion des fenêtres
- **Obligatoire** : Premier appel ncurses

#### `cbreak()`
```c
cbreak();
```
- **Rôle** : Active le mode "character break"
- **Action** : Les caractères sont disponibles immédiatement sans attendre Enter
- **Avantage** : Réactivité en temps réel pour le chat

#### `noecho()`
```c
noecho();
```
- **Rôle** : Désactive l'écho automatique des caractères tapés
- **Action** : Vous contrôlez où et comment afficher les caractères
- **Nécessaire** : Pour une interface personnalisée

#### `keypad(stdscr, TRUE)`
```c
keypad(stdscr, TRUE);
```
- **Rôle** : Active les touches spéciales (flèches, F1-F12, etc.)
- **Paramètre** : `stdscr` = écran standard, `TRUE` = activé
- **Avantage** : Navigation avec les flèches dans l'interface

#### `curs_set(1)`
```c
curs_set(1);  // 0=invisible, 1=visible, 2=très visible
```
- **Rôle** : Contrôle la visibilité du curseur
- **Options** : 0 (invisible), 1 (normal), 2 (très visible)

---

### **Gestion des Couleurs**

#### `has_colors()`
```c
if (has_colors()) {
    // Le terminal supporte les couleurs
}
```
- **Rôle** : Vérifie si le terminal supporte les couleurs
- **Retour** : TRUE si couleurs supportées, FALSE sinon
- **Sécurité** : Évite les erreurs sur terminaux monochrome

#### `start_color()`
```c
start_color();
```
- **Rôle** : Initialise le système de couleurs
- **Prérequis** : `has_colors()` doit retourner TRUE
- **Obligatoire** : Avant d'utiliser les couleurs

#### `init_pair(pair_number, foreground, background)`
```c
init_pair(1, COLOR_CYAN, COLOR_BLACK);    // Cyan sur noir
init_pair(2, COLOR_GREEN, COLOR_BLACK);   // Vert sur noir
init_pair(3, COLOR_YELLOW, COLOR_BLACK);  // Jaune sur noir
```
- **Rôle** : Définit des paires de couleurs (texte/fond)
- **Paramètres** :
  - `pair_number` : ID de la paire (1-63)
  - `foreground` : Couleur du texte
  - `background` : Couleur du fond
- **Couleurs disponibles** : `COLOR_BLACK`, `COLOR_RED`, `COLOR_GREEN`, `COLOR_YELLOW`, `COLOR_BLUE`, `COLOR_MAGENTA`, `COLOR_CYAN`, `COLOR_WHITE`

#### `wattron()` / `wattroff()`
```c
wattron(window, COLOR_PAIR(1));   // Active la paire de couleurs 1
wprintw(window, "Texte coloré");
wattroff(window, COLOR_PAIR(1));  // Désactive la couleur
```
- **Rôle** : Active/désactive les attributs de couleur sur une fenêtre
- **Usage** : Encadrer le texte à colorer

---

### **Gestion des Fenêtres**

#### `getmaxyx(window, rows, cols)`
```c
int rows, cols;
getmaxyx(stdscr, rows, cols);  // Récupère taille de l'écran
```
- **Rôle** : Récupère les dimensions d'une fenêtre
- **Paramètres** : `window`, variables pour stocker hauteur et largeur
- **Usage** : Calcul de layout responsif

#### `newwin(height, width, start_y, start_x)`
```c
WINDOW *msg_win = newwin(20, 80, 0, 0);  // 20x80 à partir de (0,0)
```
- **Rôle** : Crée une nouvelle fenêtre
- **Paramètres** :
  - `height` : Hauteur en lignes
  - `width` : Largeur en colonnes
  - `start_y` : Position Y (ligne de départ)
  - `start_x` : Position X (colonne de départ)
- **Retour** : Pointeur vers la fenêtre créée

#### `delwin(window)`
```c
delwin(msg_win);
```
- **Rôle** : Supprime une fenêtre et libère sa mémoire
- **Obligatoire** : Pour éviter les fuites mémoire

---

### **Affichage et Texte**

#### `wprintw(window, format, ...)`
```c
wprintw(msg_win, "[%s] %s: %s\n", time_str, user, message);
```
- **Rôle** : Affiche du texte formaté dans une fenêtre (comme printf)
- **Paramètres** : Fenêtre cible + format printf
- **Avantage** : Formatage facile avec variables

#### `mvwprintw(window, y, x, format, ...)`
```c
mvwprintw(window, 0, 2, " Messages ");  // À la ligne 0, colonne 2
```
- **Rôle** : Déplace le curseur ET affiche du texte
- **Combinaison** : `wmove()` + `wprintw()` en un appel
- **Usage** : Positionnement précis de texte

#### `mvwaddch(window, y, x, character)`
```c
mvwaddch(input_win, 1, input_pos, ch);  // Ajoute caractère à position
```
- **Rôle** : Déplace le curseur et ajoute UN caractère
- **Usage** : Saisie caractère par caractère dans l'input

#### `mvwdelch(window, y, x)`
```c
mvwdelch(input_win, 1, input_pos);  // Supprime caractère à position
```
- **Rôle** : Supprime le caractère à la position donnée
- **Usage** : Gestion de la touche Backspace

---

### **Gestion des Entrées**

#### `wgetch(window)`
```c
int ch = wgetch(input_win);  // Lit un caractère depuis input_win
```
- **Rôle** : Lit un caractère depuis une fenêtre spécifique
- **Retour** : Code ASCII du caractère ou code de touche spéciale
- **Blocant** : Attend qu'une touche soit pressée

#### Codes de Touches Spéciales
```c
switch (ch) {
    case KEY_UP:        // Flèche haut
    case KEY_DOWN:      // Flèche bas
    case KEY_LEFT:      // Flèche gauche
    case KEY_RIGHT:     // Flèche droite
    case KEY_BACKSPACE: // Backspace
    case KEY_ENTER:     // Entrée
    case '\n':          // Nouvelle ligne
    case '\r':          // Retour chariot
    case 127:           // Delete/Backspace alternatif
}
```

---

### **Mise à Jour et Rafraîchissement**

#### `wrefresh(window)`
```c
wrefresh(msg_win);    // Met à jour msg_win à l'écran
wrefresh(input_win);  // Met à jour input_win à l'écran
```
- **Rôle** : Affiche réellement les changements à l'écran
- **Obligatoire** : ncurses utilise un buffer, il faut "flusher"
- **Performance** : Évite les scintillements en groupant les changements

#### `refresh()`
```c
refresh();  // Équivalent à wrefresh(stdscr)
```
- **Rôle** : Met à jour l'écran standard
- **Usage** : Quand on travaille sur `stdscr` directement

---

### **Manipulation du Contenu**

#### `werase(window)`
```c
werase(status_win);  // Efface tout le contenu de status_win
```
- **Rôle** : Efface tout le contenu d'une fenêtre
- **Action** : Remplit la fenêtre d'espaces
- **Usage** : Nettoyage avant nouvel affichage

#### `box(window, vertical_char, horizontal_char)`
```c
box(msg_win, 0, 0);  // Dessine bordure avec caractères par défaut
box(msg_win, '|', '-');  // Bordure personnalisée
```
- **Rôle** : Dessine une bordure autour de la fenêtre
- **Paramètres** : 
  - `0, 0` : Utilise les caractères par défaut
  - Sinon : Caractères personnalisés pour vertical/horizontal

#### `scrollok(window, TRUE)`
```c
scrollok(msg_win, TRUE);  // Active le défilement automatique
```
- **Rôle** : Active le défilement automatique quand la fenêtre est pleine
- **Usage** : Essentiel pour la fenêtre des messages du chat
- **Comportement** : Nouvelles lignes poussent les anciennes vers le haut

---

### **Nettoyage et Sortie**

#### `endwin()`
```c
endwin();  // Restaure le terminal normal
```
- **Rôle** : Ferme ncurses et restaure le terminal normal
- **Obligatoire** : À appeler avant la fin du programme
- **Action** : Inverse de `initscr()`

---

## 🔧 Fonctions pthread Utilisées

### `pthread_create(thread, attr, start_routine, arg)`
```c
pthread_create(&client->thread, NULL, handle_client, client);
```
- **Rôle** : Crée un nouveau thread
- **Paramètres** :
  - `thread` : Pointeur vers pthread_t
  - `attr` : Attributs (NULL = défaut)
  - `start_routine` : Fonction à exécuter
  - `arg` : Argument à passer à la fonction

### `pthread_mutex_lock()` / `pthread_mutex_unlock()`
```c
pthread_mutex_lock(&clients_mutex);
// Code critique ici
pthread_mutex_unlock(&clients_mutex);
```
- **Rôle** : Synchronisation pour éviter les conditions de course
- **Usage** : Protection des variables partagées entre threads

### `pthread_detach(thread)`
```c
pthread_detach(accept_thread);
```
- **Rôle** : Le thread se nettoie automatiquement à sa fin
- **Avantage** : Pas besoin de `pthread_join()`

---

## 🕐 Fonctions time.h Utilisées

### `time(NULL)`
```c
time_t now = time(NULL);
```
- **Rôle** : Récupère le timestamp Unix actuel
- **Usage** : Horodatage des messages

### `localtime(time_t*)`
```c
struct tm *tm_info = localtime(&now);
```
- **Rôle** : Convertit timestamp en structure de temps local
- **Usage** : Formatage de l'heure lisible

### `strftime(buffer, size, format, tm)`
```c
strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
```
- **Rôle** : Formate la date/heure en chaîne
- **Format** : `%H:%M:%S` = "14:30:25"

---

## 💡 Conseils d'Utilisation

### Pattern Typique d'Affichage
```c
// 1. Modifier le contenu
wprintw(window, "Nouveau texte");

// 2. Rafraîchir pour afficher
wrefresh(window);
```

### Pattern de Nettoyage
```c
// 1. Effacer
werase(window);

// 2. Redessiner la bordure
box(window, 0, 0);

// 3. Ajouter nouveau contenu
mvwprintw(window, 0, 2, " Titre ");

// 4. Rafraîchir
wrefresh(window);
```

### Gestion Robuste des Couleurs
```c
if (has_colors()) {
    start_color();
    init_pair(1, COLOR_CYAN, COLOR_BLACK);
    wattron(window, COLOR_PAIR(1));
}
wprintw(window, "Texte");
if (has_colors()) {
    wattroff(window, COLOR_PAIR(1));
}
```

Cette documentation couvre toutes les fonctions ajoutées dans le code amélioré. Chaque fonction a un rôle spécifique dans la création d'une interface TUI moderne et fonctionnelle.