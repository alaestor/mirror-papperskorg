local ct = require("ct")

return ct.table({
  version = 52,
  lua_script = ct.file("scripts-legacy/global.lua"),
  entries = {
    ct.aa({
      id = 327,
      description = "print hello world",
      script = ct.file("scripts-legacy/327-print-hello-world.cea"),
    }),
    ct.group({
      id = 11,
      description = "",
      options = { hide_children = true, allow_manual_collapse = true },
      children = {
        ct.aa({
          id = 304,
          description = "debug: print monoclasses",
          script = ct.file("scripts-legacy/304-debug-print-monoclasses.cea"),
        }),
        ct.group({
          id = 386,
          description = "header template",
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
        }),
      },
    }),
    ct.aa({
      id = 299,
      options = { hide_children = true, allow_manual_collapse = true },
      description = "Enable",
      script = ct.file("scripts-legacy/299-enable.cea"),
      children = {
        ct.group({
          id = 230,
          description = "-=-=-=- Character",
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
          children = {
            ct.aa({
              id = 266,
              description = "immuneToDamage",
              script = ct.file("scripts-legacy/266-immunetodamage.cea"),
            }),
            ct.aa({
              id = 265,
              description = "Godmode",
              script = ct.file("scripts-legacy/265-godmode.cea"),
            }),
            ct.aa({
              id = 270,
              description = "Walking",
              script = ct.file("scripts-legacy/270-walking.cea"),
            }),
            ct.value({
              id = 269,
              description = "Walking Speed",
              value_type = ct.types.float,
              address = "mc.Player.static.instance",
              offsets = { "mc.UnitPathFinder.offset.walkspeed", "mc.Player.offset.pfinder" },
            }),
            ct.value({
              id = 249,
              description = "Health",
              value_type = ct.types.float,
              address = "mc.Player.static.instance",
              offsets = { "mc.Unit.offset.currentHealth" },
            }),
            ct.value({
              id = 240,
              description = "Special",
              value_type = ct.types.float,
              address = "mc.Player.static.instance",
              offsets = { "mc.Unit.offset.currentSpecialPower" },
            }),
            ct.value({
              id = 247,
              description = "Mana",
              value_type = ct.types.float,
              address = "mc.Player.static.instance",
              offsets = { "mc.Unit.offset.currentMana" },
            }),
            ct.aa({
              id = 358,
              description = "Instant Cooldowns",
              script = ct.file("scripts-legacy/358-instant-cooldowns.cea"),
            }),
            ct.group({
              id = 400,
              description = "Appearance",
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              children = {
                ct.value({
                  id = 401,
                  description = "Walking Speed",
                  value_type = ct.types.float,
                  address = "mc.Player.static.instance",
                  offsets = { "mc.UnitPathFinder.offset.walkspeed", "mc.Player.offset.pfinder" },
                }),
              },
            }),
          },
        }),
        ct.group({
          id = 231,
          description = "-=-=-=- Account",
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
          children = {
            ct.value({
              id = 252,
              description = "Name Len",
              value_type = ct.types.int32,
              address = "mc.Player.static.instance",
              offsets = { "0x10", "mc.Player.offset.name" },
            }),
            ct.value({
              id = 253,
              description = "Name",
              value_type = ct.types.string,
              address = "mc.Player.static.instance",
              offsets = { "0x14", "mc.Player.offset.name" },
              length = 10,
              unicode = true,
              code_page = 0,
              zero_terminate = true,
            }),
            ct.aa({
              id = 303,
              options = { allow_manual_collapse = true },
              description = "Skill EXP Totals",
              script = ct.file("scripts-legacy/303-skill-exp-totals.cea"),
            }),
            ct.aa({
              id = 308,
              options = { hide_children = true, allow_manual_collapse = true },
              description = "Stats",
              script = ct.file("scripts-legacy/308-stats.cea"),
              children = {
                ct.aa({
                  id = 317,
                  options = { allow_manual_collapse = true },
                  description = "Raw Stat Values",
                  script = ct.file("scripts-legacy/317-raw-stat-values.cea"),
                }),
                ct.aa({
                  id = 319,
                  options = { hide_children = true, allow_manual_collapse = true },
                  description = "Bonus Stat Hack",
                  script = ct.file("scripts-legacy/319-bonus-stat-hack.cea"),
                  children = {
                    ct.aa({
                      id = 322,
                      options = { allow_manual_collapse = true },
                      description = "Apply Bonuses",
                      script = ct.file("scripts-legacy/322-apply-bonuses.cea"),
                    }),
                  },
                }),
              },
            }),
            ct.aa({
              id = 237,
              options = { hide_children = true, allow_manual_collapse = true },
              description = "XP Multiplier",
              script = ct.file("scripts-legacy/237-xp-multiplier.cea"),
              children = {
                ct.value({
                  id = 238,
                  description = "value",
                  value_type = ct.types.double,
                  address = "cfg_Tdouble_multiplier_xp",
                }),
              },
            }),
          },
        }),
        ct.aa({
          id = 313,
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
          description = "-=-=-=- Inventory",
          script = ct.file("scripts-legacy/313-inventory.cea"),
          children = {
            ct.aa({
              id = 314,
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              description = "Specific Inventory Slots",
              script = ct.file("scripts-legacy/314-specific-inventory-slots.cea"),
              children = {
                ct.value({
                  id = 333,
                  description = "Slot Number (1-35)",
                  value_type = ct.types.int32,
                  address = "ui_inventory_slotNumberSelection",
                }),
                ct.aa({
                  id = 332,
                  description = "Add",
                  script = ct.file("scripts-legacy/332-add.cea"),
                }),
                ct.group({
                  id = 334,
                  description = "-------/ Inventory Slots \\-------",
                  options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
                }),
                ct.group({
                  id = 342,
                  description = "   -------\\ Inventory Slots /-------",
                }),
              },
            }),
            ct.value({
              id = 323,
              options = { manual_expand_collapse = true, allow_manual_collapse = true },
              description = "Selected Item",
              value_type = ct.types.int32,
              address = "PlayerInventorySelectedSlot",
              offsets = { "mc['UI_Inventory+ItemSlot'].offset.itemId" },
              dropdown = { items = { { value = "0", description = "placeholder" } }, description_only = true, display_value_as_item = true },
              children = {
                ct.value({
                  id = 324,
                  description = "ID#",
                  value_type = ct.types.int32,
                  address = "PlayerInventorySelectedSlot",
                  offsets = { "mc['UI_Inventory+ItemSlot'].offset.itemId" },
                }),
                ct.value({
                  id = 328,
                  description = "Quantity",
                  value_type = ct.types.int32,
                  address = "PlayerInventorySelectedSlot",
                  offsets = { "mc['UI_Inventory+ItemSlot'].offset.itemAmount" },
                }),
                ct.aa({
                  id = 343,
                  description = "Duplicate",
                  script = ct.file("scripts-legacy/343-duplicate.cea"),
                }),
                ct.aa({
                  id = 311,
                  description = "Max Stack",
                  script = ct.file("scripts-legacy/311-max-stack.cea"),
                }),
              },
            }),
          },
        }),
        ct.group({
          id = 229,
          description = "-=-=-=- Situational",
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
          children = {
            ct.value({
              id = 248,
              description = "Character Creation Trait Points",
              value_type = ct.types.int32,
              address = "mc.UI_CharacterCreation.static.instance",
              offsets = { "mc.UI_CharacterCreation.offset.traitPointsLeft" },
            }),
          },
        }),
        ct.aa({
          id = 306,
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
          description = "-=-=-=- Teleports",
          script = ct.file("scripts-legacy/306-teleports.cea"),
          children = {
            ct.aa({
              id = 347,
              description = "Save/Recall",
              script = ct.file("scripts-legacy/347-save-recall.cea"),
            }),
            ct.aa({
              id = 356,
              description = "Disable animation",
              script = ct.file("scripts-legacy/356-disable-animation.cea"),
            }),
            ct.aa({
              id = 349,
              description = "Save position as a custom teleport",
              script = ct.file("scripts-legacy/349-save-position-as-a-custom-teleport.cea"),
            }),
            ct.group({
              id = 350,
              description = "-------/ Custom Teleports \\-------",
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              children = {
                ct.group({
                  id = 352,
                  description = "- custom teleports will persist if you save the table :)",
                }),
                ct.group({
                  id = 354,
                  description = "- Feel free to move or organize them into subheaders",
                }),
                ct.aa({
                  id = 355,
                  options = { allow_manual_collapse = true },
                  description = "Mossneedle Pond Spawn",
                  script = ct.file("scripts-legacy/355-mossneedle-pond-spawn.cea"),
                }),
              },
            }),
            ct.group({
              id = 351,
              description = "   -------\\ Custom Teleports /-------",
            }),
          },
        }),
        ct.group({
          id = 369,
          description = "-=-=-=- Camera",
          options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
          children = {
            ct.group({
              id = 384,
              description = "Zoom",
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              children = {
                ct.value({
                  id = 374,
                  description = "Zoom",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.zoom" },
                }),
                ct.value({
                  id = 377,
                  description = "Zoom actual",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.zoom" },
                }),
                ct.value({
                  id = 375,
                  description = "Zoom Speed",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.zoomSpeed" },
                }),
                ct.value({
                  id = 371,
                  description = "Zoom minimum",
                  value_type = ct.types.int32,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.minZoom" },
                }),
                ct.value({
                  id = 372,
                  description = "Zoom maximum",
                  value_type = ct.types.int32,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.maxZoom" },
                }),
              },
            }),
            ct.group({
              id = 385,
              description = "Rotation",
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              children = {
                ct.value({
                  id = 376,
                  description = "Y rotation",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.rotationY" },
                }),
                ct.value({
                  id = 388,
                  description = "X rotation",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.rotationX" },
                }),
                ct.value({
                  id = 387,
                  description = "X rotation minimum",
                  value_type = ct.types.int32,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.minXRot" },
                }),
                ct.value({
                  id = 373,
                  description = "X rotation maximum",
                  value_type = ct.types.int32,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.maxXRot" },
                }),
              },
            }),
            ct.group({
              id = 398,
              description = "View distance",
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              children = {
                ct.value({
                  id = 378,
                  description = "View distance",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.viewDistance" },
                }),
                ct.value({
                  id = 379,
                  description = "View distance multiplier",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.CBCameraController.offset.viewDistanceMultiplier" },
                }),
              },
            }),
            ct.group({
              id = 390,
              description = "Post processing",
              options = { hide_children = true, manual_expand_collapse = true, allow_manual_collapse = true },
              children = {
                ct.aa({
                  id = 393,
                  description = "Active",
                  script = ct.file("scripts-legacy/393-active.cea"),
                }),
                ct.aa({
                  id = 397,
                  description = "Reset",
                  script = ct.file("scripts-legacy/397-reset.cea"),
                }),
                ct.value({
                  id = 392,
                  description = "Saturation",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.PostProcessingChanger.offset.saturation", "mc.CBCameraController.offset.postprocessing" },
                }),
                ct.value({
                  id = 395,
                  description = "Focus distance",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.PostProcessingChanger.offset.focusDistance", "mc.CBCameraController.offset.postprocessing" },
                }),
                ct.value({
                  id = 394,
                  description = "Ambient occlusion intensity",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.PostProcessingChanger.offset.aoIntensity", "mc.CBCameraController.offset.postprocessing" },
                }),
                ct.value({
                  id = 396,
                  description = "Ambient occlusion radius",
                  value_type = ct.types.float,
                  address = "mc.CBCameraController.static.instance",
                  offsets = { "mc.PostProcessingChanger.offset.aoRadius", "mc.CBCameraController.offset.postprocessing" },
                }),
              },
            }),
          },
        }),
        ct.group({
          id = 370,
          description = "",
        }),
      },
    }),
  },
})
