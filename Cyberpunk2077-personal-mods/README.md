# Alaestor's Cyberpunk 2077 Mods

A monorepo for my various personal mods.

No support or warranty. De facto abandonware. 

---

## Mods

### Precursor Cyberware

Cheaty cyberware (`precursor_cw`)

- Sandevistan `Items.PrecursorSandevistan`
  
  - Time almost stands still
  
  - Long duration
  
  - Instant cooldown
  
  - Instant recharge

- Cyberdeck `Items.PrecursorCyberdeck`
  
  - Advanced built-in quickhacks
  
  - Nearly instantaneous ram recharge
  
  - Enhances quickhack damage (experimental)
  
  - Enhances spread mechanics (experimental)
  
  - Resists being hacked (experimental)
  
  - Resists being traced (experimental)

## Notes

### _my_personal_mod_list.md

I use this markdown file to keep track of mods for my personal game.

### Binary Files

I'll try to avoid tracking binary files as much as possible. As such, `archive/` files will be stored as `raw/` text JSON which will need to be converted by Wolvenkit before a mod is packed and can be used.

The most common case will be needing to convert/import `raw/mod/ops/translation_strings.json.json` to `archive/mod/ops/translation_strings.json`.

When it's unavoidable (in cases of sound files, textures, meshes, ...), I'll use LFS.
