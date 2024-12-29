package.path = package.path .. ';./build/scripts/?.lua'

local bh = require("bh")

-- local model_id = bh.load_gltf("./resources/model1k.glb")

-- if model_id then
--     print("Loaded model with ID: ", model_id)
-- end

bh.register_function(bh.FunctionType.PRE_RENDER, function()
    -- local pos = bh.create_vec3(0, 0, 0)
    -- local size = bh.create_vec3(10, 10, 10)
    -- bh.draw_cube(pos, size, bh.Colors.RED)
   
    -- local position = {x = 0, y = 0, z = 0}        -- Position of the model in the world
    -- local scale = {x = 1, y = 1, z = 1}           -- Uniform scaling
    -- local rotation = {x = 0, y = 0, z = 0, w = 1} -- No rotation (identity quaternion)
    -- bh.draw_model(model_id, position, scale, rotation)

    -- local position = {x = 0, y = 1, z = 0}        -- Position of the model in the world
    -- local scale = {x = 1, y = 1, z = 1}           -- Uniform scaling
    -- local rotation = {x = 0, y = 0, z = 0, w = 1} -- No rotation (identity quaternion)
    -- bh.draw_model(atat, position, scale, rotation)
    
end)

-- local current_frame = 0
-- local total_frames = bh.anim_total_frames(atat, 1)

bh.register_function(bh.FunctionType.UPDATE, function(dt)
    -- bh.lupdate_animation(atat, 1, current_frame)
    -- current_frame = current_frame + 1
    
    -- if current_frame > total_frames then
    --     current_frame = 0
    -- end
end)

bh.register_function(bh.FunctionType.POST_RENDER, function()

    -- if GameScenes[bh.current_scene] then
    --     GameScenes[bh.current_scene]()
    -- else
    --     print("Failed to find bh.current_scene")
    --     GameScenes[bh.GameScenes.MAIN_MENU]()
    -- end

    

    -- local score = bh.get_player_score(_G.player)
    
    -- local player_data = bh.get_entity_data(_G.player)
    -- if not player_data then return end
    
    -- local position = string.format("Player pos: %f,%f", player_data['position']['x'], player_data['position']['y'])
    -- local pos = bh.create_vec2(10, 30)
    -- bh.draw_text(position, pos, 20, bh.Colors.GRAY)

    -- -- Create a 20 digit long string (left pad the score)
    -- local score_string = string.format("Score: \n%020d", math.floor(score * 100))
    -- local pos = bh.create_vec2(_G.screen['x']-250, 10)

    -- -- Draw the score at the top right corner
    -- bh.draw_text(score_string, pos, 20, bh.Colors.GRAY)

    -- local power = bh.get_player_power(_G.player)
    -- local health = player_data['health']
    -- if not power or not health then return end

    -- local power_string = string.format("Power: %d", math.floor(power))
    -- pos = bh.create_vec2(_G.screen['x']-250, 50)
    -- bh.draw_text(power_string, pos, 20, bh.Colors.GRAY)
    
    -- local health_string = string.format("Health: %d", math.floor(health))
    -- pos = bh.create_vec2(_G.screen['x']-250, 70)
    -- bh.draw_text(health_string, pos, 20, bh.Colors.GRAY)

end)