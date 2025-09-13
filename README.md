# bare-md4c

Fast markdown push parser.

```bash
npm i bare-md4c
```

## API

```js
import md4c from 'bare-md4c'
```

### `md4c.parse(input, onparse, flags, logDebug = false)`

Parse text `input`.

- `flags` - see exported `md4c.constants.flags`, default: `DIALECT_GITHUB`
- `logDebug` enable md4c's debug logger

Example `callback`:

```js
function onpush(event, tag, text, details) {
  const { events, block, span, text } = md4c.constants

  console.log(
    event,  // value in `events`
    tag,    // value in `block|span|text`
    text,   // the message (`undefined` on non `events.TEXT` events)
    details // tag details (header level, link href)
  )

  return 0 // continue. `-1` to abort parsing
}
```

## License

Apache-2.0
