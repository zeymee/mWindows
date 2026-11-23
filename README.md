# mWindows

Bibliothèque C++ légère qui encapsule une fenêtre native Windows (Win32) et expose les entrées utilisateur et les événements de fenêtre sous forme de callbacks.

## Fonctionnalités

### Fenêtre
- Création d'une fenêtre Win32 (`WS_OVERLAPPEDWINDOW`) à partir d'une largeur, d'une hauteur et d'un titre (`mWindow(width, height, title)`).
- Affichage automatique de la fenêtre à sa création.
- Boucle de messages non bloquante via `pullEvents()`, qui retourne `false` lorsque l'application doit se fermer (`WM_QUIT`).
- Accès au handle natif (`HWND` / `HINSTANCE`) via `getWindowHandle()`.

### Événements de fenêtre (callbacks)
Enregistrement d'une fonction de rappel par type d'événement avec `AddEventWindowCallback<T>(callback, pUserData)` :
- `CloseCallback` : demande de fermeture (`WM_CLOSE`). Si un callback est enregistré, la fermeture par défaut est remplacée.

> **Note :** un callback n'est appelé que si un `pUserData` non nul lui est fourni. Avec `nullptr` (valeur par défaut), il est ignoré.
- `FocusCallback` / `LostFocusCallback` : prise et perte du focus.
- `MinimizeCallback` : fenêtre réduite.
- `MaximizeCallback` : fenêtre agrandie, avec largeur et hauteur.
- `RestoreCallback` : fenêtre restaurée, avec largeur et hauteur.
- `MoveCallback` : déplacement de la fenêtre.

### Entrées utilisateur
- Entrées souris via **Raw Input** (`WM_INPUT`) : déplacement relatif (deltas X/Y) et molette, accessibles via `getUserInput().mouseInput`.
- Position du curseur relative à la fenêtre, mise à jour à chaque `pullEvents()`.

### Informations écran et fenêtre
- `getMonitorInfo()` : moniteur contenant la fenêtre (résolution, position, nom du périphérique, ratio d'aspect, DPI et fréquence de rafraîchissement).
- `getWindowState()` : structure d'état de la fenêtre (état et position/dimensions).

## Exemple

```cpp
#include "mWindow.hpp"

int main()
{
    mWindow window(800, 600, L"Ma fenêtre");

    while (window.pullEvents())
    {
        const auto& input = window.getUserInput();
        int32_t dX, dY;
        input.mouseInput.getDelta(dX, dY);
        // ...
    }
    return 0;
}
```

## Compilation

Le projet utilise CMake (voir [CMakeLists.txt](CMakeLists.txt) et [test/CMakeLists.txt](test/CMakeLists.txt)). Il nécessite un compilateur C++20 pour Windows.