return {
    records = {
        {
            kind = "header",
            description = "Generated table",
            children = {
                {
                    kind = "aa",
                    description = "Embedded script",
                    script = __EMBED_FILE__("scripts/embedded.cea"),
                },
            },
        },
    },
}
