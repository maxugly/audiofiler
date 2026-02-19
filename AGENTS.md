# Documentation Standards

This project uses Doxygen for documentation.

## Groups
Use the following groups to categorize classes:
* `@defgroup UI UI Components`
* `@defgroup AudioEngine Audio Engine`
* `@defgroup State State Management`
* `@defgroup Threading Threading/Workers`
* `@defgroup Helpers Helpers`

## Guidelines
*   **Headers (.h)**: Use `/** ... */` for high-level architectural documentation. Include `@brief`, `@details`, `@see`, and `@ingroup`.
*   **Implementations (.cpp)**: Use standard `//` comments for implementation details.
*   **Call Graphs**: Document complex logic flows (e.g., `AudioPlayer::getNextAudioBlock`) in the header using `@details` so they appear in Doxygen charts.
*   **Collaboration**: Document member variables (especially pointers/references to other major components) to ensure Collaboration Diagrams are accurate.

### Example for Member Variables
```cpp
/** @brief Reference to the shared application state. */
SessionState& sessionState;
```
