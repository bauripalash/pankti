#let horizontalrule = line(start: (25%,0%), end: (75%,0%))

#let cfg = (
  // Fonts
  body-font:  ("Noto Serif Bengali", "Noto Serif"),
  sans-font:  ("Noto Sans Bengali",  "Noto Sans"),
  mono-font:  ("JetBrains Mono",     "Fira Code", "Noto Sans Bengali"),

  // Sizes
  body-size:  11pt,
  h1-size:    20pt,
  h2-size:    15pt,
  h3-size:    13pt,
  h4-size:    11pt,
  code-size:  9pt,
  small-size: 9pt,

  // Colours
  text-color:    rgb("#1A1A1A"),
  muted-color:   rgb("#666666"),
  link-color:    rgb("#2563EB"),
  code-bg:       rgb("#F4F4F4"),
  rule-color:    rgb("#DDDDDD"),
  accent:        rgb("#2563EB"),  // heading underline + code left border

  // Page
  paper:         "a4",
  margin-top:    2.5cm,
  margin-bottom: 2.8cm,
  margin-left:   2.8cm,
  margin-right:  2.2cm,

  // Header / footer text
  header-left:   "পঙক্তি প্রোগ্রামিং ভাষা",
  site-url:      "pankti.palashbauri.in",
)

#set page(
  paper:  cfg.paper,
  margin: (
    top:    cfg.margin-top,
    bottom: cfg.margin-bottom,
    left:   cfg.margin-left,
    right:  cfg.margin-right,
  ),
)

#set text(
  font:   cfg.body-font,
  size:   cfg.body-size,
  fill:   cfg.text-color,
  lang:   "bn",
  region: "BD",
)

#set par(
  justify: true,
  leading: 0.9em,
  spacing: 1.4em,
)

#show table: set table(
  stroke: 0.5pt,
  columns: auto,
)

#show table.cell: set table.cell(
  inset: 4pt,
)

#show table.header: set text(weight: "bold")
