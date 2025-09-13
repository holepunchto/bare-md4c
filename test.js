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
