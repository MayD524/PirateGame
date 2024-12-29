package.path = package.path .. ';./build/scripts/?.lua'

local bh = require("bh")

bh.register_function(bh.FunctionType.ON_START, function ()
    _G.screen = bh.get_screen_size()
    _G.timer  = 0
    _G.ttest  = 0
end)

bh.register_function(bh.FunctionType.POST_RENDER, function()
    -- local entity = bh.get_entity_by_tag("hello")

    -- if not bh.is_entity_active(entity) then
    --     return
    -- end

    -- local e_data = bh.get_entity_data(entity)
    -- local p_data = bh.get_entity_data(_G.player)

    -- if p_data == nil or e_data == nil then
    --     return
    -- end
    

    -- local p_center = bh.create_vec2(
    --     p_data["position"]["x"] - (p_data["texture"]["width"] / 2),
    --     p_data["position"]["y"] - (p_data["texture"]["height"] / 2)
    -- )

    
    -- local e_center = bh.create_vec2(
    --     e_data["position"]["x"] - (e_data["texture"]["width"] / 2),
    --     e_data["position"]["y"] - (e_data["texture"]["height"] / 2)
    -- )

    -- bh.draw_line(p_center, e_center, bh.Colors.RED, 100)

end)

bh.register_function(bh.FunctionType.UPDATE, function (delta_time) 

    _G.timer = _G.timer + delta_time
end)
