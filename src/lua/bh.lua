local M = {}

-- Development flag
local is_development = false -- Set to true for development, false for production

--#region enum types
-- Functiont types
M.FunctionType = {
    UPDATE = "update",
    PRE_RENDER = "pre-render",
    POST_RENDER = "post-render",
    ON_DESTROY = "on-destroy",
    ON_START = "on-start",
}

M.GameScenes = {
    MAIN_MENU = 0
}

M.current_scene = M.GameScenes.MAIN_MENU

--#endregion

--#region Constants

--#endregion

--#region dummy functions

-- Dummy functions for development (intellisense support)
local function dummy_register_function(eventType, func) end
local function dummy_get_entity_count() end
local function dummy_create_entity(entityName, modelPaht, startPos , startVelocity, priority, maxLifeTime, scale, health) end
local function dummy_get_entity(index) end
local function dummy_print_movement(entity) end
local function dummy_are_entities_colliding(entityA, entityB) end
local function dummy_print_tags(entity) end
local function dummy_add_tag(entity, tag) end
local function dummy_get_tags(entity) end
local function dummy_remove_tag_at(entity, index) end
local function dummy_remove_tag(entity, tag) end
local function dummy_has_tag(entity, tag) end
local function dummy_remove_all_tags(entity) end
local function dummy_load_texture(texturePath) end
local function dummy_get_texture(textureID) end
local function dummy_get_position(entity) end
local function dummy_set_position(entity, x, y, z) end
local function dummy_set_velocity(entity, velocity) end
local function dummy_get_velocity(entity) end
local function dummy_get_entity_by_name(name) end
local function dummy_deal_damage(entity, damage) end
local function dummy_set_entity_active(entity, state) end
local function dummy_is_entity_active(entity) end
local function dummy_heal_entity(entity, amount) end
local function dummy_get_entity_health(entity) end
local function dummy_set_entity_rotation(entity, rotation) end
local function dummy_get_entity_rotation(entity) end
local function dummy_get_entity_data(entity) end
local function dummy_get_screen_size() end

-- local function dummy_draw_sprite(lua_State *L) end
-- local function dummy_draw_sprite_batch(lua_State *L) end
-- local function dummy_draw_texture_batch(lua_State *L) end

-- drawing functions
local function dummy_draw_text(text, pos, scale, color) end
local function dummy_draw_texture(texture, pos, rotation, scale, tint) end
local function dummy_draw_line(v_start, v_end, color, thickness) end
local function dummy_draw_rectangle(top_left, bottom_right, color, thickness, filled) end
local function dummy_draw_triangle(v1, v2, v3, color, thickness, filled) end
local function dummy_draw_circle(center, radius, color, thickness, filled) end

local function dummy_draw_cube(position, size, color) end

local function dummy_load_gltf(file_path) end
local function dummy_unload_model(model_id) end
local function dummy_draw_model(model_id, position, scale, rotation) end
local function dummy_begin_draw3d() end
local function dummy_end_draw3d() end
local function dummy_anim_total_frames(model_id, animation_index) end
local function dummy_model_total_anims(model_id) end
local function dummy_update_animation(model_id, animation_index, frame) end
local function dummy_update_animation_looping(model_id, animation_index, frame) end
--#endregion

--#region Module functions
if is_development then
    M.get_entity_count          = dummy_get_entity_count
    M.register_function         = dummy_register_function
    M.create_entity             = dummy_create_entity
    M.get_entity                = dummy_get_entity
    M.print_movement            = dummy_print_movement
    M.are_entities_colliding    = dummy_are_entities_colliding
    M.print_tags                = dummy_print_tags
    M.add_tag                   = dummy_add_tag
    M.get_tags                  = dummy_get_tags
    M.remove_tag_at             = dummy_remove_tag_at
    M.remove_tag                = dummy_remove_tag
    M.has_tag                   = dummy_has_tag
    M.remove_all_tags           = dummy_remove_all_tags
    M.load_texture              = dummy_load_texture
    M.get_texture               = dummy_get_texture
    M.get_position              = dummy_get_position
    M.set_position              = dummy_set_position
    M.get_entity_by_name        = dummy_get_entity_by_name
    M.deal_damage               = dummy_deal_damage
    M.set_entity_active         = dummy_set_entity_active
    M.is_entity_active          = dummy_is_entity_active
    M.heal_entity               = dummy_heal_entity
    M.get_entity_health         = dummy_get_entity_health
    M.draw_text                 = dummy_draw_text
    M.draw_texture              = dummy_draw_texture
    M.draw_line                 = dummy_draw_line
    M.draw_rectangle            = dummy_draw_rectangle
    M.draw_triangle             = dummy_draw_triangle
    M.draw_circle               = dummy_draw_circle
    M.draw_cube                 = dummy_draw_cube
    M.set_velocity              = dummy_set_velocity
    M.get_velocity              = dummy_get_velocity
    M.set_entity_rotation       = dummy_set_entity_rotation
    M.get_entity_rotation       = dummy_get_entity_rotation
    M.get_entity_data           = dummy_get_entity_data
    M.get_screen_size           = dummy_get_screen_size
    M.load_gltf                = dummy_load_gltf
    M.unload_model             = dummy_unload_model
    M.draw_model               = dummy_draw_model
    M.begin_draw3d             = dummy_begin_draw3d
    M.end_draw3d               = dummy_end_draw3d
    M.model_total_anims        = dummy_model_total_anims
    M.anim_total_frames        = dummy_anim_total_frames
    M.update_animation         = dummy_update_animation
    M.lupdate_animation        = dummy_update_animation_looping
else
    -- Reference the global environment (actual functions registered by C)
    M.register_function         = _G.register_function
    M.create_entity             = _G.create_entity
    M.get_entity                = _G.get_entity
    M.print_movement            = _G.print_movement
    M.are_entities_colliding    = _G.are_entities_colliding
    M.print_tags                = _G.print_tags
    M.add_tag                   = _G.add_tag
    M.get_tags                  = _G.get_tags
    M.remove_tag_at             = _G.remove_tag_at
    M.remove_tag                = _G.remove_tag
    M.has_tag                   = _G.has_tag
    M.remove_all_tags           = _G.remove_all_tags
    M.load_texture              = _G.load_texture
    M.get_texture               = _G.get_texture
    M.get_entity_count          = _G.get_entity_count
    M.get_position              = _G.get_position
    M.set_position              = _G.set_position
    M.get_entity_by_name        = _G.get_entity_by_name
    M.deal_damage               = _G.deal_damage
    M.set_entity_active         = _G.set_entity_active
    M.is_entity_active          = _G.is_entity_active                 
    M.heal_entity               = _G.heal_entity                              
    M.get_entity_health         = _G.get_entity_health      
    M.draw_text                 = _G.draw_text
    M.draw_texture              = _G.draw_texture
    M.draw_line                 = _G.draw_line
    M.draw_rectangle            = _G.draw_rectangle
    M.draw_triangle             = _G.draw_triangle
    M.draw_circle               = _G.draw_circle       
    M.draw_cube                 = _G.draw_cube
    M.set_velocity              = _G.set_velocity
    M.get_velocity              = _G.get_velocity
    M.set_entity_rotation       = _G.set_entity_rotation
    M.get_entity_rotation       = _G.get_entity_rotation
    M.get_entity_data           = _G.get_entity_data
    M.get_screen_size           = _G.get_screen_size
    M.load_gltf                = _G.load_gltf
    M.unload_model             = _G.unload_model
    M.draw_model               = _G.draw_model
    M.begin_draw3d             = _G.begin_draw3d
    M.end_draw3d               = _G.end_draw3d
    M.model_total_anims        = _G.model_total_anims
    M.anim_total_frames        = _G.anim_total_frames
    M.update_animation         = _G.update_animation
    M.lupdate_animation        = _G.lupdate_animation
end
--#endregion

--#region Entity controls

---Get the first entity with a tag
---@param tag string
---@return nil | userdata
M.get_entity_by_tag = function(tag)
    local num_entities = M.get_entity_count()
    for i = 0, num_entities - 1 do
        local entity = M.get_entity(i)
        if M.has_tag(entity, tag) and M.is_entity_active(entity) then
            return entity
        end
    end

    return nil
end

---Get an array of all entities with a specific tag
---@param tag string
---@return table
M.get_all_entities_with_tag = function(tag)
    local entity_list = {}
    local num_entities = M.get_entity_count()

    for i = 0, num_entities-1 do
        local entity = M.get_entity(i)
        if M.has_tag(entity, tag) and M.is_entity_active(entity) then
            table.insert(entity_list, entity)
        end
    end
    return entity_list
end

--#endregion

--#region colors
---Create a color table
---@param r number
---@param g number
---@param b number
---@param a number|nil
---@return table
M.create_color = function (r, g, b, a)
    a = a or 255
    return {
        r = r,
        g = g,
        b = b,
        a = a
    }
end

M.Colors = {
    GRAY = M.create_color(130, 130, 130),
    RED = M.create_color(255, 0, 0),
    GREEN = M.create_color(0, 255, 0),
    BLUE = M.create_color(0, 0, 255),
    WHITE = M.create_color(255, 255, 255),
    BLACK = M.create_color(0, 0, 0),
}
--#endregion

--#region Vectors
---Create a vector 2
---@param x number
---@param y number
---@return table
M.create_vec2 = function(x, y)
    return {
        x = x, y = y
    }
end

M.create_vec3 = function(x, y, z)
    return {
        x = x, y=y, z=z
    }
end

M.VEC2_ZERO = M.create_vec2(0, 0)
M.VEC2_ONE  = M.create_vec2(1, 1)
--#endregion


--#region Drawing shapes
M.draw_rectangle_lines = function (top_left, bottom_right, color, thickness)
    M.draw_rectangle(top_left, bottom_right, color, thickness, false)
end

M.draw_rectangle_fill = function (top_left, bottom_right, color, thickness) 
    M.draw_rectangle(top_left, bottom_right, color, thickness, true)
end

M.draw_circle_lines = function (center, radius, color, thickness) 
    M.draw_circle(center, radius, color, thickness, true)
end

M.draw_circle_fill = function (center, radius, color, thickness) 
    M.draw_circle(center, radius, color, thickness, true)
end

M.draw_triangle_lines = function (v1, v2, v3, color, thickness)
    M.draw_triangle(v1, v2, v3, color, thickness, false)
end

M.draw_triangle_fill = function (v1, v2, v3, color, thickness) 
    M.draw_triangle(v1, v2, v3, color, thickness, true)
end
--#endregion


--#region useful functions
M.dump = function (o, indent)
    indent = indent or 0
    local indent_str = string.rep("  ", indent)
    if type(o) == 'table' then
        local s = '{\n'
        for k, v in pairs(o) do
            if type(k) ~= 'number' then k = '"'..k..'"' end
            s = s .. indent_str .. '  ['..k..'] = ' .. M.dump(v, indent + 1) .. ',\n'
        end
        return s .. indent_str .. '}'
    else
        return tostring(o)
    end
end

M.protect = function(table)
    return setmetatable({}, { 
        __index = table, 
        __newindex = function(table, key, value) 
            print("Error writing to '"..table.."' tried to modify '"..key.."' to '"..value.."' but was unable to due to this table being readonly...")
            error("Attempt to modify read-only table") 
        end, 
        __metatable = false 
    })
end

M.tablelen = function (T)
    local count = 0
    for _ in pairs(T) do count = count + 1 end
    return count
end

-- Define the ShapeType enum
M.ShapeType = {
    RECTANGLE = 0,
    CIRCLE = 1,
    TRIANGLE = 2
}

-- Function to convert ShapeType to string
local function shapeTypeToString(shapeType)
    if shapeType == M.ShapeType.RECTANGLE then
        return "RECT"
    elseif shapeType == M.ShapeType.CIRCLE then
        return "CIRC"
    elseif shapeType == M.ShapeType.TRIANGLE then
        return "TRI"
    else
        return "UNKNOWN"
    end
end

M.create_texture_str =  function (shapeType, position1, position2, position3, color, thickness, filled)
    local shapeStr = shapeTypeToString(shapeType)
    local filledStr = filled and "true" or "false"
    local result = ""

    if shapeType == M.ShapeType.RECTANGLE then
        result = string.format("SHAPE:%s:%.2f,%.2f:%.2f,%.2f:%.2f,%.2f:%d,%d,%d,%d:%d:%s",
            shapeStr, position1.x, position1.y, position2.x, position2.y, 0, 0,
            color.r, color.g, color.b, color.a, thickness, filledStr)
    elseif shapeType == M.ShapeType.CIRCLE then
        result = string.format("SHAPE:%s:%.2f,%.2f:%.2f,%.2f:%.2f,%.2f:%d,%d,%d,%d:%d:%s",
            shapeStr, position1.x, position1.y, position2.x, 0, 0, 0, 
            color.r, color.g, color.b, color.a, thickness, filledStr)
    elseif shapeType == M.ShapeType.TRIANGLE then
        result = string.format("SHAPE:%s:%.2f,%.2f:%.2f,%.2f:%.2f,%.2f:%d,%d,%d,%d:%d:%s",
            shapeStr, position1.x, position1.y, position2.x, position2.y, 
            position3.x, position3.y, color.r, color.g, color.b, color.a, thickness, filledStr)
    end

    return result
end

M.get_direction_vector = function(v1, v2, speed)
    return M.create_vec2((v1['x'] - v2['x']) * speed['x'], (v1['y'] - v2['y']) * speed['y'])
end


--#endregion

return M