const test = require('brittle')
const md4c = require('.')

const fixture = require('./fixture.md', { with: { type: 'text' } })

test('parses commonmark spec', (t) => {
  let events = 0

  md4c.parse(fixture, onpush, 0)
  t.is(events, 17878)

  function onpush(event, tag, text, details) {
    events++

    return 0
  }
})

const htmlFixture = `
# Hello World

welcome to planet.
`.trim()

test('generates html', (t) => {
  const html = md4c.toHTML(htmlFixture)
  t.is(html, '<h1>Hello World</h1>\n<p>welcome to planet.</p>\n')
})
