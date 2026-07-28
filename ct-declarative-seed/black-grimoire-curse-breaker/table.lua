return {
    records = {
        {
            kind = "aa",
            description = "print hello world",
            script = __EMBED_FILE__("print-hello-world.cea"),
        },
        {
            kind = "aa",
            description = "Enable",
            script = __EMBED_FILE__("enable.cea"),
            properties = {
                Options = "",
            },
            children = {
                {
                    kind = "header",
                    description = "-=-=-=- Character",
                    properties = {
                        Options = "",
                    },
                    children = {
                        {
                            kind = "aa",
                            description = "immuneToDamage",
                            script = __EMBED_FILE__("enable-character-immunetodamage.cea"),
                        },
                        {
                            kind = "aa",
                            description = "Godmode",
                            script = __EMBED_FILE__("enable-character-godmode.cea"),
                        },
                        {
                            kind = "aa",
                            description = "Walking",
                            script = __EMBED_FILE__("enable-character-walking.cea"),
                        },
                        {
                            kind = "record",
                            description = "Walking Speed",
                            address = "mc.Player.static.instance",
                            vtype = vtSingle,
                            offsets = { "mc.UnitPathFinder.offset.walkspeed", "mc.Player.offset.pfinder" },
                            properties = {
                                ShowAsSigned = false,
                            },
                        },
                        {
                            kind = "record",
                            description = "Health",
                            address = "mc.Player.static.instance",
                            vtype = vtSingle,
                            offsets = { "mc.Unit.offset.currentHealth" },
                            properties = {
                                ShowAsSigned = false,
                            },
                        },
                        {
                            kind = "record",
                            description = "Special",
                            address = "mc.Player.static.instance",
                            vtype = vtSingle,
                            offsets = { "mc.Unit.offset.currentSpecialPower" },
                            properties = {
                                ShowAsSigned = false,
                            },
                        },
                        {
                            kind = "record",
                            description = "Mana",
                            address = "mc.Player.static.instance",
                            vtype = vtSingle,
                            offsets = { "mc.Unit.offset.currentMana" },
                            properties = {
                                ShowAsSigned = false,
                            },
                        },
                        {
                            kind = "aa",
                            description = "Instant Cooldowns",
                            script = __EMBED_FILE__("enable-character-instant-cooldowns.cea"),
                        },
                        {
                            kind = "header",
                            description = "Appearance",
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "Walking Speed",
                                    address = "mc.Player.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.UnitPathFinder.offset.walkspeed", "mc.Player.offset.pfinder" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                            },
                        },
                    },
                },
                {
                    kind = "header",
                    description = "-=-=-=- Account",
                    properties = {
                        Options = "",
                    },
                    children = {
                        {
                            kind = "record",
                            description = "Name Len",
                            address = "mc.Player.static.instance",
                            vtype = vtDword,
                            offsets = { "alce.mono.T.string.length", "mc.Player.offset.name" },
                            properties = {
                                ShowAsSigned = false,
                            },
                        },
                        {
                            kind = "record",
                            description = "Name",
                            address = "mc.Player.static.instance",
                            vtype = vtString,
                            offsets = { "alce.mono.T.string.index_from", "mc.Player.offset.name" },
                            properties = {
                                ShowAsSigned = false,
                                ["String.Size"] = 10,
                                ["String.Unicode"] = true,
                                ["String.Codepage"] = false,
                            },
                        },
                        {
                            kind = "aa",
                            description = "Skill EXP Totals",
                            script = __EMBED_FILE__("enable-account-skill-exp-totals.cea"),
                            properties = {
                                Options = "",
                            },
                        },
                        {
                            kind = "aa",
                            description = "Stats",
                            script = __EMBED_FILE__("enable-account-stats.cea"),
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "aa",
                                    description = "Raw Stat Values",
                                    script = __EMBED_FILE__("enable-account-stats-raw-stat-values.cea"),
                                    properties = {
                                        Options = "",
                                    },
                                },
                                {
                                    kind = "aa",
                                    description = "Bonus Stat Hack",
                                    script = __EMBED_FILE__("enable-account-stats-bonus-stat-hack.cea"),
                                    properties = {
                                        Options = "",
                                    },
                                    children = {
                                        {
                                            kind = "aa",
                                            description = "Apply Bonuses",
                                            script = __EMBED_FILE__("enable-account-stats-bonus-stat-hack-apply-bonuses.cea"),
                                            properties = {
                                                Options = "",
                                            },
                                        },
                                    },
                                },
                            },
                        },
                        {
                            kind = "aa",
                            description = "XP Multiplier",
                            script = __EMBED_FILE__("enable-account-xp-multiplier.cea"),
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "value",
                                    address = "cfg_Tdouble_multiplier_xp",
                                    vtype = vtDouble,
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                            },
                        },
                    },
                },
                {
                    kind = "aa",
                    description = "-=-=-=- Inventory",
                    script = __EMBED_FILE__("enable-inventory.cea"),
                    properties = {
                        Options = "",
                    },
                    children = {
                        {
                            kind = "aa",
                            description = "Specific Inventory Slots",
                            script = __EMBED_FILE__("enable-inventory-specific-inventory-slots.cea"),
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "Slot Number (1-35)",
                                    address = "ui_inventory_slotNumberSelection",
                                    vtype = vtDword,
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "aa",
                                    description = "Add",
                                    script = __EMBED_FILE__("enable-inventory-specific-inventory-slots-add.cea"),
                                },
                                {
                                    kind = "header",
                                    description = "-------/ Inventory Slots \\-------",
                                    properties = {
                                        Options = "",
                                    },
                                },
                                {
                                    kind = "header",
                                    description = "   -------\\ Inventory Slots /-------",
                                },
                            },
                        },
                        {
                            kind = "record",
                            description = "Selected Item",
                            address = "PlayerInventorySelectedSlot",
                            vtype = vtDword,
                            offsets = { "mc['UI_Inventory+ItemSlot'].offset.itemId" },
                            properties = {
                                ShowAsSigned = false,
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "ID#",
                                    address = "PlayerInventorySelectedSlot",
                                    vtype = vtDword,
                                    offsets = { "mc['UI_Inventory+ItemSlot'].offset.itemId" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Quantity",
                                    address = "PlayerInventorySelectedSlot",
                                    vtype = vtDword,
                                    offsets = { "mc['UI_Inventory+ItemSlot'].offset.itemAmount" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "aa",
                                    description = "Duplicate",
                                    script = __EMBED_FILE__("enable-inventory-selected-item-duplicate.cea"),
                                },
                                {
                                    kind = "aa",
                                    description = "Max Stack",
                                    script = __EMBED_FILE__("enable-inventory-selected-item-max-stack.cea"),
                                },
                            },
                        },
                    },
                },
                {
                    kind = "header",
                    description = "-=-=-=- Situational",
                    properties = {
                        Options = "",
                    },
                    children = {
                        {
                            kind = "record",
                            description = "Character Creation Trait Points",
                            address = "mc.UI_CharacterCreation.static.instance",
                            vtype = vtDword,
                            offsets = { "mc.UI_CharacterCreation.offset.traitPointsLeft" },
                            properties = {
                                ShowAsSigned = false,
                            },
                        },
                    },
                },
                {
                    kind = "aa",
                    description = "-=-=-=- Teleports",
                    script = __EMBED_FILE__("enable-teleports.cea"),
                    properties = {
                        Options = "",
                    },
                    children = {
                        {
                            kind = "aa",
                            description = "Save/Recall",
                            script = __EMBED_FILE__("enable-teleports-save-recall.cea"),
                        },
                        {
                            kind = "aa",
                            description = "Disable animation",
                            script = __EMBED_FILE__("enable-teleports-disable-animation.cea"),
                        },
                        {
                            kind = "aa",
                            description = "Save position as a custom teleport",
                            script = __EMBED_FILE__("enable-teleports-save-position-as-a-custom-teleport.cea"),
                        },
                        {
                            kind = "header",
                            description = "-------/ Custom Teleports \\-------",
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "header",
                                    description = "- custom teleports will persist if you save the table :)",
                                },
                                {
                                    kind = "header",
                                    description = "- Feel free to move or organize them into subheaders",
                                },
                                {
                                    kind = "aa",
                                    description = "Mossneedle Pond Spawn",
                                    script = __EMBED_FILE__("enable-teleports-custom-teleports-mossneedle-pond-spawn.cea"),
                                    properties = {
                                        Options = "",
                                    },
                                },
                            },
                        },
                        {
                            kind = "header",
                            description = "   -------\\ Custom Teleports /-------",
                        },
                    },
                },
                {
                    kind = "header",
                    description = "-=-=-=- Camera",
                    properties = {
                        Options = "",
                    },
                    children = {
                        {
                            kind = "header",
                            description = "Zoom",
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "Zoom",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.zoom" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Zoom actual",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.zoom" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Zoom Speed",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.zoomSpeed" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Zoom minimum",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtDword,
                                    offsets = { "mc.CBCameraController.offset.minZoom" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Zoom maximum",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtDword,
                                    offsets = { "mc.CBCameraController.offset.maxZoom" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                            },
                        },
                        {
                            kind = "header",
                            description = "Rotation",
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "Y rotation",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.rotationY" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "X rotation",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.rotationX" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "X rotation minimum",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtDword,
                                    offsets = { "mc.CBCameraController.offset.minXRot" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "X rotation maximum",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtDword,
                                    offsets = { "mc.CBCameraController.offset.maxXRot" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                            },
                        },
                        {
                            kind = "header",
                            description = "View distance",
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "record",
                                    description = "View distance",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.viewDistance" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "View distance multiplier",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.CBCameraController.offset.viewDistanceMultiplier" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                            },
                        },
                        {
                            kind = "header",
                            description = "Post processing",
                            properties = {
                                Options = "",
                            },
                            children = {
                                {
                                    kind = "aa",
                                    description = "Active",
                                    script = __EMBED_FILE__("enable-camera-post-processing-active.cea"),
                                },
                                {
                                    kind = "aa",
                                    description = "Reset",
                                    script = __EMBED_FILE__("enable-camera-post-processing-reset.cea"),
                                },
                                {
                                    kind = "record",
                                    description = "Saturation",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.PostProcessingChanger.offset.saturation", "mc.CBCameraController.offset.postprocessing" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Focus distance",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.PostProcessingChanger.offset.focusDistance", "mc.CBCameraController.offset.postprocessing" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Ambient occlusion intensity",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.PostProcessingChanger.offset.aoIntensity", "mc.CBCameraController.offset.postprocessing" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                                {
                                    kind = "record",
                                    description = "Ambient occlusion radius",
                                    address = "mc.CBCameraController.static.instance",
                                    vtype = vtSingle,
                                    offsets = { "mc.PostProcessingChanger.offset.aoRadius", "mc.CBCameraController.offset.postprocessing" },
                                    properties = {
                                        ShowAsSigned = false,
                                    },
                                },
                            },
                        },
                    },
                },
                {
                    kind = "header",
                    description = "",
                },
            },
        },
    },
}
