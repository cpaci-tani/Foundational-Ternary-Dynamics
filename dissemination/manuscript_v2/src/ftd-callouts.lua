-- ftd-callouts.lua
-- Maps custom FTD callout types to tcolorbox environments in PDF output

local callout_map = {
  ["callout-axiom"] = "ftdaxiom",
  ["callout-theorem"] = "ftdtheorem",
  ["callout-definition"] = "ftddefinition",
  ["callout-selection"] = "ftdselection",
  ["callout-conjecture"] = "ftdconjecture",
}

function Div(el)
  -- Only process for LaTeX/PDF output
  if not quarto.doc.is_format("pdf") and not quarto.doc.is_format("latex") then
    return el
  end

  for class, env in pairs(callout_map) do
    if el.classes:includes(class) then
      -- Extract title from the first Header element
      local title_inlines = nil
      local body_blocks = {}
      local found_header = false

      for _, block in ipairs(el.content) do
        if not found_header and block.t == "Header" then
          title_inlines = block.content
          found_header = true
        else
          table.insert(body_blocks, block)
        end
      end

      -- Render title inlines to LaTeX (preserves math mode)
      local title_latex = ""
      if title_inlines then
        local doc = pandoc.Pandoc({pandoc.Plain(title_inlines)})
        title_latex = pandoc.write(doc, "latex")
        -- Trim whitespace
        title_latex = title_latex:gsub("^%s+", ""):gsub("%s+$", "")
      end

      -- Build the LaTeX wrapper
      local open_env = pandoc.RawBlock("latex",
        "\\begin{" .. env .. "}[" .. title_latex .. "]")
      local close_env = pandoc.RawBlock("latex",
        "\\end{" .. env .. "}")

      local result = {open_env}
      for _, block in ipairs(body_blocks) do
        table.insert(result, block)
      end
      table.insert(result, close_env)

      return result
    end
  end

  return el
end
