---@class alce global namespace for the ALCE library
local alce = require("globals")

alce.cheat_table = require("cheat_table")
alce.fmt = require("fmt")
alce.globals = require("globals")
alce.memory = require("memory")
alce.mono = require("mono")
alce.mono_plumbing = require("mono_plumbing")
alce.mono_t = require("mono_t")
alce.monoscript = require("monoscript")
alce.printers = require("printers")
for name, printer in pairs(alce.printers) do
    if type(printer) == "function" then
        alce[name] = printer
    end
end
alce.T = require("t")
alce.utils = require("utils")
alce.validators = require("validators")
alce.vt = require("vt")

return alce
