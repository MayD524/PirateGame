package.path = package.path .. ';D:/bullet_hell/build/scripts/?.lua'

local bh = require("bh")

bh.register_function(bh.FunctionType.ON_START, function ()

end)

bh.register_function(bh.FunctionType.POST_RENDER, function()

end)

bh.register_function(bh.FunctionType.UPDATE, function (delta_time) 

end)

bh.register_function(bh.FunctionType.ON_DESTROY, function ()

end)