# Multijugador en red: Actividad 2

Utilizando alguno de los proyectos realizados en clase para el sistema multijugador en red de Unreal Engine 4 (el proyecto realizado con blueprints, o el proyecto realizado en C++), añade la siguiente mecánica multijugador:

- La mecánica se basa en un nuevo actor que deberás implementar con una malla estática y caja de colisión que debe ubicarse en la escena. Es un actor  replicable con proxies para todos los clientes.
- El actor representa un hito en el juego (por ejemplo, premio, puerta para pasar al siguiente nivel, etcétera). El hito se consigue cuando al menos dos jugadores se solapan simultáneamente en la caja de colisión.
- Cuando el hito se consigue, el actor debe modificar su aspecto (por ejemplo instancia dinámica de material) y/o realizar el spawn  de un efecto (sistema de partículas). Estos cambios deben ser visibles para todos los jugadores.
- No es necesario implementar nada más, en relación al hito (ni cambios en el hud, ni cambios de nivel ni nada por el estilo), solo lo mencionado en los tres apartados anteriores.
