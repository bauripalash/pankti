#let horizontalrule = line(start: (25%,0%), end: (75%,0%))

#let cfg = (
  // Fonts
  body-font:  ("Noto Serif Bengali", "Noto Serif"),
  sans-font:  ("Noto Sans Bengali",  "Noto Sans"),
  mono-font:  ("Noto Sans Bengali" ,"JetBrains Mono", "Fira Code"),

  // Sizes
  body-size:  12pt,
  h1-size:    20pt,
  h2-size:    15pt,
  h3-size:    13pt,
  h4-size:    11pt,
  code-size:  11pt,
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
  header: context {
    if counter(page).get().first() > 1 {
      set text(size: cfg.small-size, fill: cfg.muted-color, font: cfg.sans-font)
      grid(
        columns: (1fr, auto),
        cfg.header-left,
      )
      v(-6pt)
      line(length: 100%, stroke: 0.5pt + cfg.rule-color)
    }
  },
  footer: context {
    if counter(page).get().first() > 1 {
      line(length: 100%, stroke: 0.5pt + cfg.rule-color)
      v(-6pt)
      set text(size: cfg.small-size, fill: cfg.muted-color, font: cfg.sans-font)
      align(center)[#counter(page).display("1 / 1", both: true)]
    }
  },
)


#set text(
  font:   cfg.body-font,
  size:   cfg.body-size,
  fill:   cfg.text-color,
  lang:   "bn",
  region: "IN",
)

#set par(
  justify: true,
  leading: 0.9em,
  spacing: 1.4em,
)

#show raw.where(block: true): it => {
  v(0.5em)
  block(
    width:  100%,
    fill:   cfg.code-bg,
    inset:  (left: 12pt, right: 12pt, top: 10pt, bottom: 10pt),
    radius: 2pt,
  )[
    #set text(font: cfg.body-font, size: cfg.code-size, fill: cfg.text-color)
    #set par(justify: false, leading: 1.1em)
    #it
  ]
  v(0.5em)
}

#show raw.where(block: false): it => {
  box(
    fill:   cfg.code-bg,
    stroke: 0.5pt + cfg.rule-color,
    inset:  (x: 4pt, y: 2pt),
    radius: 2pt,
  )[
    #text(font: cfg.body-font, size: cfg.code-size)[#it]
  ]
}

#show table: set table(
  stroke: 0.5pt,
  columns: auto,
)

#show table.cell: set table.cell(
  inset: 4pt,
)

#show table.header: set text(weight: "bold")
