#include <land.h>

t_Entity create_land(Vector3 position, Vector2 size, Color color) {
    t_Entity land = {0};

    land.entity_name = "Land";
    land.is_active = true;
    land.layer = 0;
    land.entity3D.position = position;
    land.entity3D.scale = (Vector3){size.x, 1.0f, size.y};
    land.entity_data = malloc(sizeof(BuildingData));

    if (land.entity_data) {
        BuildingData* data = (BuildingData*)land.entity_data;
        data->width = size.x;
        data->depth = size.y;
        data->color = color;
        data->is_collidable = true;
    }

    land.update = NULL; // Land doesn't need updates
    // land.on_destroy = destroy_land;

    return land;
}

void render_land(t_Entity* entity) {
    if (!entity || !entity->is_active) return;

    BuildingData* data = (BuildingData*)entity->entity_data;
    Vector3 position = entity->entity3D.position;
    Vector2 size = (Vector2){entity->entity3D.scale.x, entity->entity3D.scale.z};

    DrawPlane(position, size, data->color);
}
