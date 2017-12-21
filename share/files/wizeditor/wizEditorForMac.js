(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof require=="function"&&require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof require=="function"&&require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(require,module,exports){
'use strict'

exports.byteLength = byteLength
exports.toByteArray = toByteArray
exports.fromByteArray = fromByteArray

var lookup = []
var revLookup = []
var Arr = typeof Uint8Array !== 'undefined' ? Uint8Array : Array

var code = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'
for (var i = 0, len = code.length; i < len; ++i) {
  lookup[i] = code[i]
  revLookup[code.charCodeAt(i)] = i
}

revLookup['-'.charCodeAt(0)] = 62
revLookup['_'.charCodeAt(0)] = 63

function placeHoldersCount (b64) {
  var len = b64.length
  if (len % 4 > 0) {
    throw new Error('Invalid string. Length must be a multiple of 4')
  }

  // the number of equal signs (place holders)
  // if there are two placeholders, than the two characters before it
  // represent one byte
  // if there is only one, then the three characters before it represent 2 bytes
  // this is just a cheap hack to not do indexOf twice
  return b64[len - 2] === '=' ? 2 : b64[len - 1] === '=' ? 1 : 0
}

function byteLength (b64) {
  // base64 is 4/3 + up to two characters of the original data
  return b64.length * 3 / 4 - placeHoldersCount(b64)
}

function toByteArray (b64) {
  var i, j, l, tmp, placeHolders, arr
  var len = b64.length
  placeHolders = placeHoldersCount(b64)

  arr = new Arr(len * 3 / 4 - placeHolders)

  // if there are placeholders, only get up to the last complete 4 chars
  l = placeHolders > 0 ? len - 4 : len

  var L = 0

  for (i = 0, j = 0; i < l; i += 4, j += 3) {
    tmp = (revLookup[b64.charCodeAt(i)] << 18) | (revLookup[b64.charCodeAt(i + 1)] << 12) | (revLookup[b64.charCodeAt(i + 2)] << 6) | revLookup[b64.charCodeAt(i + 3)]
    arr[L++] = (tmp >> 16) & 0xFF
    arr[L++] = (tmp >> 8) & 0xFF
    arr[L++] = tmp & 0xFF
  }

  if (placeHolders === 2) {
    tmp = (revLookup[b64.charCodeAt(i)] << 2) | (revLookup[b64.charCodeAt(i + 1)] >> 4)
    arr[L++] = tmp & 0xFF
  } else if (placeHolders === 1) {
    tmp = (revLookup[b64.charCodeAt(i)] << 10) | (revLookup[b64.charCodeAt(i + 1)] << 4) | (revLookup[b64.charCodeAt(i + 2)] >> 2)
    arr[L++] = (tmp >> 8) & 0xFF
    arr[L++] = tmp & 0xFF
  }

  return arr
}

function tripletToBase64 (num) {
  return lookup[num >> 18 & 0x3F] + lookup[num >> 12 & 0x3F] + lookup[num >> 6 & 0x3F] + lookup[num & 0x3F]
}

function encodeChunk (uint8, start, end) {
  var tmp
  var output = []
  for (var i = start; i < end; i += 3) {
    tmp = (uint8[i] << 16) + (uint8[i + 1] << 8) + (uint8[i + 2])
    output.push(tripletToBase64(tmp))
  }
  return output.join('')
}

function fromByteArray (uint8) {
  var tmp
  var len = uint8.length
  var extraBytes = len % 3 // if we have 1 byte left, pad 2 bytes
  var output = ''
  var parts = []
  var maxChunkLength = 16383 // must be multiple of 3

  // go through the array every three bytes, we'll deal with trailing stuff later
  for (var i = 0, len2 = len - extraBytes; i < len2; i += maxChunkLength) {
    parts.push(encodeChunk(uint8, i, (i + maxChunkLength) > len2 ? len2 : (i + maxChunkLength)))
  }

  // pad the end with zeros, but make sure to not forget the extra bytes
  if (extraBytes === 1) {
    tmp = uint8[len - 1]
    output += lookup[tmp >> 2]
    output += lookup[(tmp << 4) & 0x3F]
    output += '=='
  } else if (extraBytes === 2) {
    tmp = (uint8[len - 2] << 8) + (uint8[len - 1])
    output += lookup[tmp >> 10]
    output += lookup[(tmp >> 4) & 0x3F]
    output += lookup[(tmp << 2) & 0x3F]
    output += '='
  }

  parts.push(output)

  return parts.join('')
}

},{}],2:[function(require,module,exports){
(function (global){
/*!
 * The buffer module from node.js, for the browser.
 *
 * @author   Feross Aboukhadijeh <feross@feross.org> <http://feross.org>
 * @license  MIT
 */
/* eslint-disable no-proto */

'use strict'

var base64 = require('base64-js')
var ieee754 = require('ieee754')
var isArray = require('isarray')

exports.Buffer = Buffer
exports.SlowBuffer = SlowBuffer
exports.INSPECT_MAX_BYTES = 50

/**
 * If `Buffer.TYPED_ARRAY_SUPPORT`:
 *   === true    Use Uint8Array implementation (fastest)
 *   === false   Use Object implementation (most compatible, even IE6)
 *
 * Browsers that support typed arrays are IE 10+, Firefox 4+, Chrome 7+, Safari 5.1+,
 * Opera 11.6+, iOS 4.2+.
 *
 * Due to various browser bugs, sometimes the Object implementation will be used even
 * when the browser supports typed arrays.
 *
 * Note:
 *
 *   - Firefox 4-29 lacks support for adding new properties to `Uint8Array` instances,
 *     See: https://bugzilla.mozilla.org/show_bug.cgi?id=695438.
 *
 *   - Chrome 9-10 is missing the `TypedArray.prototype.subarray` function.
 *
 *   - IE10 has a broken `TypedArray.prototype.subarray` function which returns arrays of
 *     incorrect length in some situations.

 * We detect these buggy browsers and set `Buffer.TYPED_ARRAY_SUPPORT` to `false` so they
 * get the Object implementation, which is slower but behaves correctly.
 */
Buffer.TYPED_ARRAY_SUPPORT = global.TYPED_ARRAY_SUPPORT !== undefined
  ? global.TYPED_ARRAY_SUPPORT
  : typedArraySupport()

/*
 * Export kMaxLength after typed array support is determined.
 */
exports.kMaxLength = kMaxLength()

function typedArraySupport () {
  try {
    var arr = new Uint8Array(1)
    arr.__proto__ = {__proto__: Uint8Array.prototype, foo: function () { return 42 }}
    return arr.foo() === 42 && // typed array instances can be augmented
        typeof arr.subarray === 'function' && // chrome 9-10 lack `subarray`
        arr.subarray(1, 1).byteLength === 0 // ie10 has broken `subarray`
  } catch (e) {
    return false
  }
}

function kMaxLength () {
  return Buffer.TYPED_ARRAY_SUPPORT
    ? 0x7fffffff
    : 0x3fffffff
}

function createBuffer (that, length) {
  if (kMaxLength() < length) {
    throw new RangeError('Invalid typed array length')
  }
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    // Return an augmented `Uint8Array` instance, for best performance
    that = new Uint8Array(length)
    that.__proto__ = Buffer.prototype
  } else {
    // Fallback: Return an object instance of the Buffer class
    if (that === null) {
      that = new Buffer(length)
    }
    that.length = length
  }

  return that
}

/**
 * The Buffer constructor returns instances of `Uint8Array` that have their
 * prototype changed to `Buffer.prototype`. Furthermore, `Buffer` is a subclass of
 * `Uint8Array`, so the returned instances will have all the node `Buffer` methods
 * and the `Uint8Array` methods. Square bracket notation works as expected -- it
 * returns a single octet.
 *
 * The `Uint8Array` prototype remains unmodified.
 */

function Buffer (arg, encodingOrOffset, length) {
  if (!Buffer.TYPED_ARRAY_SUPPORT && !(this instanceof Buffer)) {
    return new Buffer(arg, encodingOrOffset, length)
  }

  // Common case.
  if (typeof arg === 'number') {
    if (typeof encodingOrOffset === 'string') {
      throw new Error(
        'If encoding is specified then the first argument must be a string'
      )
    }
    return allocUnsafe(this, arg)
  }
  return from(this, arg, encodingOrOffset, length)
}

Buffer.poolSize = 8192 // not used by this implementation

// TODO: Legacy, not needed anymore. Remove in next major version.
Buffer._augment = function (arr) {
  arr.__proto__ = Buffer.prototype
  return arr
}

function from (that, value, encodingOrOffset, length) {
  if (typeof value === 'number') {
    throw new TypeError('"value" argument must not be a number')
  }

  if (typeof ArrayBuffer !== 'undefined' && value instanceof ArrayBuffer) {
    return fromArrayBuffer(that, value, encodingOrOffset, length)
  }

  if (typeof value === 'string') {
    return fromString(that, value, encodingOrOffset)
  }

  return fromObject(that, value)
}

/**
 * Functionally equivalent to Buffer(arg, encoding) but throws a TypeError
 * if value is a number.
 * Buffer.from(str[, encoding])
 * Buffer.from(array)
 * Buffer.from(buffer)
 * Buffer.from(arrayBuffer[, byteOffset[, length]])
 **/
Buffer.from = function (value, encodingOrOffset, length) {
  return from(null, value, encodingOrOffset, length)
}

if (Buffer.TYPED_ARRAY_SUPPORT) {
  Buffer.prototype.__proto__ = Uint8Array.prototype
  Buffer.__proto__ = Uint8Array
  if (typeof Symbol !== 'undefined' && Symbol.species &&
      Buffer[Symbol.species] === Buffer) {
    // Fix subarray() in ES2016. See: https://github.com/feross/buffer/pull/97
    Object.defineProperty(Buffer, Symbol.species, {
      value: null,
      configurable: true
    })
  }
}

function assertSize (size) {
  if (typeof size !== 'number') {
    throw new TypeError('"size" argument must be a number')
  } else if (size < 0) {
    throw new RangeError('"size" argument must not be negative')
  }
}

function alloc (that, size, fill, encoding) {
  assertSize(size)
  if (size <= 0) {
    return createBuffer(that, size)
  }
  if (fill !== undefined) {
    // Only pay attention to encoding if it's a string. This
    // prevents accidentally sending in a number that would
    // be interpretted as a start offset.
    return typeof encoding === 'string'
      ? createBuffer(that, size).fill(fill, encoding)
      : createBuffer(that, size).fill(fill)
  }
  return createBuffer(that, size)
}

/**
 * Creates a new filled Buffer instance.
 * alloc(size[, fill[, encoding]])
 **/
Buffer.alloc = function (size, fill, encoding) {
  return alloc(null, size, fill, encoding)
}

function allocUnsafe (that, size) {
  assertSize(size)
  that = createBuffer(that, size < 0 ? 0 : checked(size) | 0)
  if (!Buffer.TYPED_ARRAY_SUPPORT) {
    for (var i = 0; i < size; ++i) {
      that[i] = 0
    }
  }
  return that
}

/**
 * Equivalent to Buffer(num), by default creates a non-zero-filled Buffer instance.
 * */
Buffer.allocUnsafe = function (size) {
  return allocUnsafe(null, size)
}
/**
 * Equivalent to SlowBuffer(num), by default creates a non-zero-filled Buffer instance.
 */
Buffer.allocUnsafeSlow = function (size) {
  return allocUnsafe(null, size)
}

function fromString (that, string, encoding) {
  if (typeof encoding !== 'string' || encoding === '') {
    encoding = 'utf8'
  }

  if (!Buffer.isEncoding(encoding)) {
    throw new TypeError('"encoding" must be a valid string encoding')
  }

  var length = byteLength(string, encoding) | 0
  that = createBuffer(that, length)

  var actual = that.write(string, encoding)

  if (actual !== length) {
    // Writing a hex string, for example, that contains invalid characters will
    // cause everything after the first invalid character to be ignored. (e.g.
    // 'abxxcd' will be treated as 'ab')
    that = that.slice(0, actual)
  }

  return that
}

function fromArrayLike (that, array) {
  var length = array.length < 0 ? 0 : checked(array.length) | 0
  that = createBuffer(that, length)
  for (var i = 0; i < length; i += 1) {
    that[i] = array[i] & 255
  }
  return that
}

function fromArrayBuffer (that, array, byteOffset, length) {
  array.byteLength // this throws if `array` is not a valid ArrayBuffer

  if (byteOffset < 0 || array.byteLength < byteOffset) {
    throw new RangeError('\'offset\' is out of bounds')
  }

  if (array.byteLength < byteOffset + (length || 0)) {
    throw new RangeError('\'length\' is out of bounds')
  }

  if (byteOffset === undefined && length === undefined) {
    array = new Uint8Array(array)
  } else if (length === undefined) {
    array = new Uint8Array(array, byteOffset)
  } else {
    array = new Uint8Array(array, byteOffset, length)
  }

  if (Buffer.TYPED_ARRAY_SUPPORT) {
    // Return an augmented `Uint8Array` instance, for best performance
    that = array
    that.__proto__ = Buffer.prototype
  } else {
    // Fallback: Return an object instance of the Buffer class
    that = fromArrayLike(that, array)
  }
  return that
}

function fromObject (that, obj) {
  if (Buffer.isBuffer(obj)) {
    var len = checked(obj.length) | 0
    that = createBuffer(that, len)

    if (that.length === 0) {
      return that
    }

    obj.copy(that, 0, 0, len)
    return that
  }

  if (obj) {
    if ((typeof ArrayBuffer !== 'undefined' &&
        obj.buffer instanceof ArrayBuffer) || 'length' in obj) {
      if (typeof obj.length !== 'number' || isnan(obj.length)) {
        return createBuffer(that, 0)
      }
      return fromArrayLike(that, obj)
    }

    if (obj.type === 'Buffer' && isArray(obj.data)) {
      return fromArrayLike(that, obj.data)
    }
  }

  throw new TypeError('First argument must be a string, Buffer, ArrayBuffer, Array, or array-like object.')
}

function checked (length) {
  // Note: cannot use `length < kMaxLength()` here because that fails when
  // length is NaN (which is otherwise coerced to zero.)
  if (length >= kMaxLength()) {
    throw new RangeError('Attempt to allocate Buffer larger than maximum ' +
                         'size: 0x' + kMaxLength().toString(16) + ' bytes')
  }
  return length | 0
}

function SlowBuffer (length) {
  if (+length != length) { // eslint-disable-line eqeqeq
    length = 0
  }
  return Buffer.alloc(+length)
}

Buffer.isBuffer = function isBuffer (b) {
  return !!(b != null && b._isBuffer)
}

Buffer.compare = function compare (a, b) {
  if (!Buffer.isBuffer(a) || !Buffer.isBuffer(b)) {
    throw new TypeError('Arguments must be Buffers')
  }

  if (a === b) return 0

  var x = a.length
  var y = b.length

  for (var i = 0, len = Math.min(x, y); i < len; ++i) {
    if (a[i] !== b[i]) {
      x = a[i]
      y = b[i]
      break
    }
  }

  if (x < y) return -1
  if (y < x) return 1
  return 0
}

Buffer.isEncoding = function isEncoding (encoding) {
  switch (String(encoding).toLowerCase()) {
    case 'hex':
    case 'utf8':
    case 'utf-8':
    case 'ascii':
    case 'latin1':
    case 'binary':
    case 'base64':
    case 'ucs2':
    case 'ucs-2':
    case 'utf16le':
    case 'utf-16le':
      return true
    default:
      return false
  }
}

Buffer.concat = function concat (list, length) {
  if (!isArray(list)) {
    throw new TypeError('"list" argument must be an Array of Buffers')
  }

  if (list.length === 0) {
    return Buffer.alloc(0)
  }

  var i
  if (length === undefined) {
    length = 0
    for (i = 0; i < list.length; ++i) {
      length += list[i].length
    }
  }

  var buffer = Buffer.allocUnsafe(length)
  var pos = 0
  for (i = 0; i < list.length; ++i) {
    var buf = list[i]
    if (!Buffer.isBuffer(buf)) {
      throw new TypeError('"list" argument must be an Array of Buffers')
    }
    buf.copy(buffer, pos)
    pos += buf.length
  }
  return buffer
}

function byteLength (string, encoding) {
  if (Buffer.isBuffer(string)) {
    return string.length
  }
  if (typeof ArrayBuffer !== 'undefined' && typeof ArrayBuffer.isView === 'function' &&
      (ArrayBuffer.isView(string) || string instanceof ArrayBuffer)) {
    return string.byteLength
  }
  if (typeof string !== 'string') {
    string = '' + string
  }

  var len = string.length
  if (len === 0) return 0

  // Use a for loop to avoid recursion
  var loweredCase = false
  for (;;) {
    switch (encoding) {
      case 'ascii':
      case 'latin1':
      case 'binary':
        return len
      case 'utf8':
      case 'utf-8':
      case undefined:
        return utf8ToBytes(string).length
      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return len * 2
      case 'hex':
        return len >>> 1
      case 'base64':
        return base64ToBytes(string).length
      default:
        if (loweredCase) return utf8ToBytes(string).length // assume utf8
        encoding = ('' + encoding).toLowerCase()
        loweredCase = true
    }
  }
}
Buffer.byteLength = byteLength

function slowToString (encoding, start, end) {
  var loweredCase = false

  // No need to verify that "this.length <= MAX_UINT32" since it's a read-only
  // property of a typed array.

  // This behaves neither like String nor Uint8Array in that we set start/end
  // to their upper/lower bounds if the value passed is out of range.
  // undefined is handled specially as per ECMA-262 6th Edition,
  // Section 13.3.3.7 Runtime Semantics: KeyedBindingInitialization.
  if (start === undefined || start < 0) {
    start = 0
  }
  // Return early if start > this.length. Done here to prevent potential uint32
  // coercion fail below.
  if (start > this.length) {
    return ''
  }

  if (end === undefined || end > this.length) {
    end = this.length
  }

  if (end <= 0) {
    return ''
  }

  // Force coersion to uint32. This will also coerce falsey/NaN values to 0.
  end >>>= 0
  start >>>= 0

  if (end <= start) {
    return ''
  }

  if (!encoding) encoding = 'utf8'

  while (true) {
    switch (encoding) {
      case 'hex':
        return hexSlice(this, start, end)

      case 'utf8':
      case 'utf-8':
        return utf8Slice(this, start, end)

      case 'ascii':
        return asciiSlice(this, start, end)

      case 'latin1':
      case 'binary':
        return latin1Slice(this, start, end)

      case 'base64':
        return base64Slice(this, start, end)

      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return utf16leSlice(this, start, end)

      default:
        if (loweredCase) throw new TypeError('Unknown encoding: ' + encoding)
        encoding = (encoding + '').toLowerCase()
        loweredCase = true
    }
  }
}

// The property is used by `Buffer.isBuffer` and `is-buffer` (in Safari 5-7) to detect
// Buffer instances.
Buffer.prototype._isBuffer = true

function swap (b, n, m) {
  var i = b[n]
  b[n] = b[m]
  b[m] = i
}

Buffer.prototype.swap16 = function swap16 () {
  var len = this.length
  if (len % 2 !== 0) {
    throw new RangeError('Buffer size must be a multiple of 16-bits')
  }
  for (var i = 0; i < len; i += 2) {
    swap(this, i, i + 1)
  }
  return this
}

Buffer.prototype.swap32 = function swap32 () {
  var len = this.length
  if (len % 4 !== 0) {
    throw new RangeError('Buffer size must be a multiple of 32-bits')
  }
  for (var i = 0; i < len; i += 4) {
    swap(this, i, i + 3)
    swap(this, i + 1, i + 2)
  }
  return this
}

Buffer.prototype.swap64 = function swap64 () {
  var len = this.length
  if (len % 8 !== 0) {
    throw new RangeError('Buffer size must be a multiple of 64-bits')
  }
  for (var i = 0; i < len; i += 8) {
    swap(this, i, i + 7)
    swap(this, i + 1, i + 6)
    swap(this, i + 2, i + 5)
    swap(this, i + 3, i + 4)
  }
  return this
}

Buffer.prototype.toString = function toString () {
  var length = this.length | 0
  if (length === 0) return ''
  if (arguments.length === 0) return utf8Slice(this, 0, length)
  return slowToString.apply(this, arguments)
}

Buffer.prototype.equals = function equals (b) {
  if (!Buffer.isBuffer(b)) throw new TypeError('Argument must be a Buffer')
  if (this === b) return true
  return Buffer.compare(this, b) === 0
}

Buffer.prototype.inspect = function inspect () {
  var str = ''
  var max = exports.INSPECT_MAX_BYTES
  if (this.length > 0) {
    str = this.toString('hex', 0, max).match(/.{2}/g).join(' ')
    if (this.length > max) str += ' ... '
  }
  return '<Buffer ' + str + '>'
}

Buffer.prototype.compare = function compare (target, start, end, thisStart, thisEnd) {
  if (!Buffer.isBuffer(target)) {
    throw new TypeError('Argument must be a Buffer')
  }

  if (start === undefined) {
    start = 0
  }
  if (end === undefined) {
    end = target ? target.length : 0
  }
  if (thisStart === undefined) {
    thisStart = 0
  }
  if (thisEnd === undefined) {
    thisEnd = this.length
  }

  if (start < 0 || end > target.length || thisStart < 0 || thisEnd > this.length) {
    throw new RangeError('out of range index')
  }

  if (thisStart >= thisEnd && start >= end) {
    return 0
  }
  if (thisStart >= thisEnd) {
    return -1
  }
  if (start >= end) {
    return 1
  }

  start >>>= 0
  end >>>= 0
  thisStart >>>= 0
  thisEnd >>>= 0

  if (this === target) return 0

  var x = thisEnd - thisStart
  var y = end - start
  var len = Math.min(x, y)

  var thisCopy = this.slice(thisStart, thisEnd)
  var targetCopy = target.slice(start, end)

  for (var i = 0; i < len; ++i) {
    if (thisCopy[i] !== targetCopy[i]) {
      x = thisCopy[i]
      y = targetCopy[i]
      break
    }
  }

  if (x < y) return -1
  if (y < x) return 1
  return 0
}

// Finds either the first index of `val` in `buffer` at offset >= `byteOffset`,
// OR the last index of `val` in `buffer` at offset <= `byteOffset`.
//
// Arguments:
// - buffer - a Buffer to search
// - val - a string, Buffer, or number
// - byteOffset - an index into `buffer`; will be clamped to an int32
// - encoding - an optional encoding, relevant is val is a string
// - dir - true for indexOf, false for lastIndexOf
function bidirectionalIndexOf (buffer, val, byteOffset, encoding, dir) {
  // Empty buffer means no match
  if (buffer.length === 0) return -1

  // Normalize byteOffset
  if (typeof byteOffset === 'string') {
    encoding = byteOffset
    byteOffset = 0
  } else if (byteOffset > 0x7fffffff) {
    byteOffset = 0x7fffffff
  } else if (byteOffset < -0x80000000) {
    byteOffset = -0x80000000
  }
  byteOffset = +byteOffset  // Coerce to Number.
  if (isNaN(byteOffset)) {
    // byteOffset: it it's undefined, null, NaN, "foo", etc, search whole buffer
    byteOffset = dir ? 0 : (buffer.length - 1)
  }

  // Normalize byteOffset: negative offsets start from the end of the buffer
  if (byteOffset < 0) byteOffset = buffer.length + byteOffset
  if (byteOffset >= buffer.length) {
    if (dir) return -1
    else byteOffset = buffer.length - 1
  } else if (byteOffset < 0) {
    if (dir) byteOffset = 0
    else return -1
  }

  // Normalize val
  if (typeof val === 'string') {
    val = Buffer.from(val, encoding)
  }

  // Finally, search either indexOf (if dir is true) or lastIndexOf
  if (Buffer.isBuffer(val)) {
    // Special case: looking for empty string/buffer always fails
    if (val.length === 0) {
      return -1
    }
    return arrayIndexOf(buffer, val, byteOffset, encoding, dir)
  } else if (typeof val === 'number') {
    val = val & 0xFF // Search for a byte value [0-255]
    if (Buffer.TYPED_ARRAY_SUPPORT &&
        typeof Uint8Array.prototype.indexOf === 'function') {
      if (dir) {
        return Uint8Array.prototype.indexOf.call(buffer, val, byteOffset)
      } else {
        return Uint8Array.prototype.lastIndexOf.call(buffer, val, byteOffset)
      }
    }
    return arrayIndexOf(buffer, [ val ], byteOffset, encoding, dir)
  }

  throw new TypeError('val must be string, number or Buffer')
}

function arrayIndexOf (arr, val, byteOffset, encoding, dir) {
  var indexSize = 1
  var arrLength = arr.length
  var valLength = val.length

  if (encoding !== undefined) {
    encoding = String(encoding).toLowerCase()
    if (encoding === 'ucs2' || encoding === 'ucs-2' ||
        encoding === 'utf16le' || encoding === 'utf-16le') {
      if (arr.length < 2 || val.length < 2) {
        return -1
      }
      indexSize = 2
      arrLength /= 2
      valLength /= 2
      byteOffset /= 2
    }
  }

  function read (buf, i) {
    if (indexSize === 1) {
      return buf[i]
    } else {
      return buf.readUInt16BE(i * indexSize)
    }
  }

  var i
  if (dir) {
    var foundIndex = -1
    for (i = byteOffset; i < arrLength; i++) {
      if (read(arr, i) === read(val, foundIndex === -1 ? 0 : i - foundIndex)) {
        if (foundIndex === -1) foundIndex = i
        if (i - foundIndex + 1 === valLength) return foundIndex * indexSize
      } else {
        if (foundIndex !== -1) i -= i - foundIndex
        foundIndex = -1
      }
    }
  } else {
    if (byteOffset + valLength > arrLength) byteOffset = arrLength - valLength
    for (i = byteOffset; i >= 0; i--) {
      var found = true
      for (var j = 0; j < valLength; j++) {
        if (read(arr, i + j) !== read(val, j)) {
          found = false
          break
        }
      }
      if (found) return i
    }
  }

  return -1
}

Buffer.prototype.includes = function includes (val, byteOffset, encoding) {
  return this.indexOf(val, byteOffset, encoding) !== -1
}

Buffer.prototype.indexOf = function indexOf (val, byteOffset, encoding) {
  return bidirectionalIndexOf(this, val, byteOffset, encoding, true)
}

Buffer.prototype.lastIndexOf = function lastIndexOf (val, byteOffset, encoding) {
  return bidirectionalIndexOf(this, val, byteOffset, encoding, false)
}

function hexWrite (buf, string, offset, length) {
  offset = Number(offset) || 0
  var remaining = buf.length - offset
  if (!length) {
    length = remaining
  } else {
    length = Number(length)
    if (length > remaining) {
      length = remaining
    }
  }

  // must be an even number of digits
  var strLen = string.length
  if (strLen % 2 !== 0) throw new TypeError('Invalid hex string')

  if (length > strLen / 2) {
    length = strLen / 2
  }
  for (var i = 0; i < length; ++i) {
    var parsed = parseInt(string.substr(i * 2, 2), 16)
    if (isNaN(parsed)) return i
    buf[offset + i] = parsed
  }
  return i
}

function utf8Write (buf, string, offset, length) {
  return blitBuffer(utf8ToBytes(string, buf.length - offset), buf, offset, length)
}

function asciiWrite (buf, string, offset, length) {
  return blitBuffer(asciiToBytes(string), buf, offset, length)
}

function latin1Write (buf, string, offset, length) {
  return asciiWrite(buf, string, offset, length)
}

function base64Write (buf, string, offset, length) {
  return blitBuffer(base64ToBytes(string), buf, offset, length)
}

function ucs2Write (buf, string, offset, length) {
  return blitBuffer(utf16leToBytes(string, buf.length - offset), buf, offset, length)
}

Buffer.prototype.write = function write (string, offset, length, encoding) {
  // Buffer#write(string)
  if (offset === undefined) {
    encoding = 'utf8'
    length = this.length
    offset = 0
  // Buffer#write(string, encoding)
  } else if (length === undefined && typeof offset === 'string') {
    encoding = offset
    length = this.length
    offset = 0
  // Buffer#write(string, offset[, length][, encoding])
  } else if (isFinite(offset)) {
    offset = offset | 0
    if (isFinite(length)) {
      length = length | 0
      if (encoding === undefined) encoding = 'utf8'
    } else {
      encoding = length
      length = undefined
    }
  // legacy write(string, encoding, offset, length) - remove in v0.13
  } else {
    throw new Error(
      'Buffer.write(string, encoding, offset[, length]) is no longer supported'
    )
  }

  var remaining = this.length - offset
  if (length === undefined || length > remaining) length = remaining

  if ((string.length > 0 && (length < 0 || offset < 0)) || offset > this.length) {
    throw new RangeError('Attempt to write outside buffer bounds')
  }

  if (!encoding) encoding = 'utf8'

  var loweredCase = false
  for (;;) {
    switch (encoding) {
      case 'hex':
        return hexWrite(this, string, offset, length)

      case 'utf8':
      case 'utf-8':
        return utf8Write(this, string, offset, length)

      case 'ascii':
        return asciiWrite(this, string, offset, length)

      case 'latin1':
      case 'binary':
        return latin1Write(this, string, offset, length)

      case 'base64':
        // Warning: maxLength not taken into account in base64Write
        return base64Write(this, string, offset, length)

      case 'ucs2':
      case 'ucs-2':
      case 'utf16le':
      case 'utf-16le':
        return ucs2Write(this, string, offset, length)

      default:
        if (loweredCase) throw new TypeError('Unknown encoding: ' + encoding)
        encoding = ('' + encoding).toLowerCase()
        loweredCase = true
    }
  }
}

Buffer.prototype.toJSON = function toJSON () {
  return {
    type: 'Buffer',
    data: Array.prototype.slice.call(this._arr || this, 0)
  }
}

function base64Slice (buf, start, end) {
  if (start === 0 && end === buf.length) {
    return base64.fromByteArray(buf)
  } else {
    return base64.fromByteArray(buf.slice(start, end))
  }
}

function utf8Slice (buf, start, end) {
  end = Math.min(buf.length, end)
  var res = []

  var i = start
  while (i < end) {
    var firstByte = buf[i]
    var codePoint = null
    var bytesPerSequence = (firstByte > 0xEF) ? 4
      : (firstByte > 0xDF) ? 3
      : (firstByte > 0xBF) ? 2
      : 1

    if (i + bytesPerSequence <= end) {
      var secondByte, thirdByte, fourthByte, tempCodePoint

      switch (bytesPerSequence) {
        case 1:
          if (firstByte < 0x80) {
            codePoint = firstByte
          }
          break
        case 2:
          secondByte = buf[i + 1]
          if ((secondByte & 0xC0) === 0x80) {
            tempCodePoint = (firstByte & 0x1F) << 0x6 | (secondByte & 0x3F)
            if (tempCodePoint > 0x7F) {
              codePoint = tempCodePoint
            }
          }
          break
        case 3:
          secondByte = buf[i + 1]
          thirdByte = buf[i + 2]
          if ((secondByte & 0xC0) === 0x80 && (thirdByte & 0xC0) === 0x80) {
            tempCodePoint = (firstByte & 0xF) << 0xC | (secondByte & 0x3F) << 0x6 | (thirdByte & 0x3F)
            if (tempCodePoint > 0x7FF && (tempCodePoint < 0xD800 || tempCodePoint > 0xDFFF)) {
              codePoint = tempCodePoint
            }
          }
          break
        case 4:
          secondByte = buf[i + 1]
          thirdByte = buf[i + 2]
          fourthByte = buf[i + 3]
          if ((secondByte & 0xC0) === 0x80 && (thirdByte & 0xC0) === 0x80 && (fourthByte & 0xC0) === 0x80) {
            tempCodePoint = (firstByte & 0xF) << 0x12 | (secondByte & 0x3F) << 0xC | (thirdByte & 0x3F) << 0x6 | (fourthByte & 0x3F)
            if (tempCodePoint > 0xFFFF && tempCodePoint < 0x110000) {
              codePoint = tempCodePoint
            }
          }
      }
    }

    if (codePoint === null) {
      // we did not generate a valid codePoint so insert a
      // replacement char (U+FFFD) and advance only 1 byte
      codePoint = 0xFFFD
      bytesPerSequence = 1
    } else if (codePoint > 0xFFFF) {
      // encode to utf16 (surrogate pair dance)
      codePoint -= 0x10000
      res.push(codePoint >>> 10 & 0x3FF | 0xD800)
      codePoint = 0xDC00 | codePoint & 0x3FF
    }

    res.push(codePoint)
    i += bytesPerSequence
  }

  return decodeCodePointsArray(res)
}

// Based on http://stackoverflow.com/a/22747272/680742, the browser with
// the lowest limit is Chrome, with 0x10000 args.
// We go 1 magnitude less, for safety
var MAX_ARGUMENTS_LENGTH = 0x1000

function decodeCodePointsArray (codePoints) {
  var len = codePoints.length
  if (len <= MAX_ARGUMENTS_LENGTH) {
    return String.fromCharCode.apply(String, codePoints) // avoid extra slice()
  }

  // Decode in chunks to avoid "call stack size exceeded".
  var res = ''
  var i = 0
  while (i < len) {
    res += String.fromCharCode.apply(
      String,
      codePoints.slice(i, i += MAX_ARGUMENTS_LENGTH)
    )
  }
  return res
}

function asciiSlice (buf, start, end) {
  var ret = ''
  end = Math.min(buf.length, end)

  for (var i = start; i < end; ++i) {
    ret += String.fromCharCode(buf[i] & 0x7F)
  }
  return ret
}

function latin1Slice (buf, start, end) {
  var ret = ''
  end = Math.min(buf.length, end)

  for (var i = start; i < end; ++i) {
    ret += String.fromCharCode(buf[i])
  }
  return ret
}

function hexSlice (buf, start, end) {
  var len = buf.length

  if (!start || start < 0) start = 0
  if (!end || end < 0 || end > len) end = len

  var out = ''
  for (var i = start; i < end; ++i) {
    out += toHex(buf[i])
  }
  return out
}

function utf16leSlice (buf, start, end) {
  var bytes = buf.slice(start, end)
  var res = ''
  for (var i = 0; i < bytes.length; i += 2) {
    res += String.fromCharCode(bytes[i] + bytes[i + 1] * 256)
  }
  return res
}

Buffer.prototype.slice = function slice (start, end) {
  var len = this.length
  start = ~~start
  end = end === undefined ? len : ~~end

  if (start < 0) {
    start += len
    if (start < 0) start = 0
  } else if (start > len) {
    start = len
  }

  if (end < 0) {
    end += len
    if (end < 0) end = 0
  } else if (end > len) {
    end = len
  }

  if (end < start) end = start

  var newBuf
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    newBuf = this.subarray(start, end)
    newBuf.__proto__ = Buffer.prototype
  } else {
    var sliceLen = end - start
    newBuf = new Buffer(sliceLen, undefined)
    for (var i = 0; i < sliceLen; ++i) {
      newBuf[i] = this[i + start]
    }
  }

  return newBuf
}

/*
 * Need to make sure that buffer isn't trying to write out of bounds.
 */
function checkOffset (offset, ext, length) {
  if ((offset % 1) !== 0 || offset < 0) throw new RangeError('offset is not uint')
  if (offset + ext > length) throw new RangeError('Trying to access beyond buffer length')
}

Buffer.prototype.readUIntLE = function readUIntLE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkOffset(offset, byteLength, this.length)

  var val = this[offset]
  var mul = 1
  var i = 0
  while (++i < byteLength && (mul *= 0x100)) {
    val += this[offset + i] * mul
  }

  return val
}

Buffer.prototype.readUIntBE = function readUIntBE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) {
    checkOffset(offset, byteLength, this.length)
  }

  var val = this[offset + --byteLength]
  var mul = 1
  while (byteLength > 0 && (mul *= 0x100)) {
    val += this[offset + --byteLength] * mul
  }

  return val
}

Buffer.prototype.readUInt8 = function readUInt8 (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 1, this.length)
  return this[offset]
}

Buffer.prototype.readUInt16LE = function readUInt16LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  return this[offset] | (this[offset + 1] << 8)
}

Buffer.prototype.readUInt16BE = function readUInt16BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  return (this[offset] << 8) | this[offset + 1]
}

Buffer.prototype.readUInt32LE = function readUInt32LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return ((this[offset]) |
      (this[offset + 1] << 8) |
      (this[offset + 2] << 16)) +
      (this[offset + 3] * 0x1000000)
}

Buffer.prototype.readUInt32BE = function readUInt32BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return (this[offset] * 0x1000000) +
    ((this[offset + 1] << 16) |
    (this[offset + 2] << 8) |
    this[offset + 3])
}

Buffer.prototype.readIntLE = function readIntLE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkOffset(offset, byteLength, this.length)

  var val = this[offset]
  var mul = 1
  var i = 0
  while (++i < byteLength && (mul *= 0x100)) {
    val += this[offset + i] * mul
  }
  mul *= 0x80

  if (val >= mul) val -= Math.pow(2, 8 * byteLength)

  return val
}

Buffer.prototype.readIntBE = function readIntBE (offset, byteLength, noAssert) {
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) checkOffset(offset, byteLength, this.length)

  var i = byteLength
  var mul = 1
  var val = this[offset + --i]
  while (i > 0 && (mul *= 0x100)) {
    val += this[offset + --i] * mul
  }
  mul *= 0x80

  if (val >= mul) val -= Math.pow(2, 8 * byteLength)

  return val
}

Buffer.prototype.readInt8 = function readInt8 (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 1, this.length)
  if (!(this[offset] & 0x80)) return (this[offset])
  return ((0xff - this[offset] + 1) * -1)
}

Buffer.prototype.readInt16LE = function readInt16LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  var val = this[offset] | (this[offset + 1] << 8)
  return (val & 0x8000) ? val | 0xFFFF0000 : val
}

Buffer.prototype.readInt16BE = function readInt16BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 2, this.length)
  var val = this[offset + 1] | (this[offset] << 8)
  return (val & 0x8000) ? val | 0xFFFF0000 : val
}

Buffer.prototype.readInt32LE = function readInt32LE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return (this[offset]) |
    (this[offset + 1] << 8) |
    (this[offset + 2] << 16) |
    (this[offset + 3] << 24)
}

Buffer.prototype.readInt32BE = function readInt32BE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)

  return (this[offset] << 24) |
    (this[offset + 1] << 16) |
    (this[offset + 2] << 8) |
    (this[offset + 3])
}

Buffer.prototype.readFloatLE = function readFloatLE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)
  return ieee754.read(this, offset, true, 23, 4)
}

Buffer.prototype.readFloatBE = function readFloatBE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 4, this.length)
  return ieee754.read(this, offset, false, 23, 4)
}

Buffer.prototype.readDoubleLE = function readDoubleLE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 8, this.length)
  return ieee754.read(this, offset, true, 52, 8)
}

Buffer.prototype.readDoubleBE = function readDoubleBE (offset, noAssert) {
  if (!noAssert) checkOffset(offset, 8, this.length)
  return ieee754.read(this, offset, false, 52, 8)
}

function checkInt (buf, value, offset, ext, max, min) {
  if (!Buffer.isBuffer(buf)) throw new TypeError('"buffer" argument must be a Buffer instance')
  if (value > max || value < min) throw new RangeError('"value" argument is out of bounds')
  if (offset + ext > buf.length) throw new RangeError('Index out of range')
}

Buffer.prototype.writeUIntLE = function writeUIntLE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) {
    var maxBytes = Math.pow(2, 8 * byteLength) - 1
    checkInt(this, value, offset, byteLength, maxBytes, 0)
  }

  var mul = 1
  var i = 0
  this[offset] = value & 0xFF
  while (++i < byteLength && (mul *= 0x100)) {
    this[offset + i] = (value / mul) & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeUIntBE = function writeUIntBE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  byteLength = byteLength | 0
  if (!noAssert) {
    var maxBytes = Math.pow(2, 8 * byteLength) - 1
    checkInt(this, value, offset, byteLength, maxBytes, 0)
  }

  var i = byteLength - 1
  var mul = 1
  this[offset + i] = value & 0xFF
  while (--i >= 0 && (mul *= 0x100)) {
    this[offset + i] = (value / mul) & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeUInt8 = function writeUInt8 (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 1, 0xff, 0)
  if (!Buffer.TYPED_ARRAY_SUPPORT) value = Math.floor(value)
  this[offset] = (value & 0xff)
  return offset + 1
}

function objectWriteUInt16 (buf, value, offset, littleEndian) {
  if (value < 0) value = 0xffff + value + 1
  for (var i = 0, j = Math.min(buf.length - offset, 2); i < j; ++i) {
    buf[offset + i] = (value & (0xff << (8 * (littleEndian ? i : 1 - i)))) >>>
      (littleEndian ? i : 1 - i) * 8
  }
}

Buffer.prototype.writeUInt16LE = function writeUInt16LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0xffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value & 0xff)
    this[offset + 1] = (value >>> 8)
  } else {
    objectWriteUInt16(this, value, offset, true)
  }
  return offset + 2
}

Buffer.prototype.writeUInt16BE = function writeUInt16BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0xffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 8)
    this[offset + 1] = (value & 0xff)
  } else {
    objectWriteUInt16(this, value, offset, false)
  }
  return offset + 2
}

function objectWriteUInt32 (buf, value, offset, littleEndian) {
  if (value < 0) value = 0xffffffff + value + 1
  for (var i = 0, j = Math.min(buf.length - offset, 4); i < j; ++i) {
    buf[offset + i] = (value >>> (littleEndian ? i : 3 - i) * 8) & 0xff
  }
}

Buffer.prototype.writeUInt32LE = function writeUInt32LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0xffffffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset + 3] = (value >>> 24)
    this[offset + 2] = (value >>> 16)
    this[offset + 1] = (value >>> 8)
    this[offset] = (value & 0xff)
  } else {
    objectWriteUInt32(this, value, offset, true)
  }
  return offset + 4
}

Buffer.prototype.writeUInt32BE = function writeUInt32BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0xffffffff, 0)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 24)
    this[offset + 1] = (value >>> 16)
    this[offset + 2] = (value >>> 8)
    this[offset + 3] = (value & 0xff)
  } else {
    objectWriteUInt32(this, value, offset, false)
  }
  return offset + 4
}

Buffer.prototype.writeIntLE = function writeIntLE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) {
    var limit = Math.pow(2, 8 * byteLength - 1)

    checkInt(this, value, offset, byteLength, limit - 1, -limit)
  }

  var i = 0
  var mul = 1
  var sub = 0
  this[offset] = value & 0xFF
  while (++i < byteLength && (mul *= 0x100)) {
    if (value < 0 && sub === 0 && this[offset + i - 1] !== 0) {
      sub = 1
    }
    this[offset + i] = ((value / mul) >> 0) - sub & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeIntBE = function writeIntBE (value, offset, byteLength, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) {
    var limit = Math.pow(2, 8 * byteLength - 1)

    checkInt(this, value, offset, byteLength, limit - 1, -limit)
  }

  var i = byteLength - 1
  var mul = 1
  var sub = 0
  this[offset + i] = value & 0xFF
  while (--i >= 0 && (mul *= 0x100)) {
    if (value < 0 && sub === 0 && this[offset + i + 1] !== 0) {
      sub = 1
    }
    this[offset + i] = ((value / mul) >> 0) - sub & 0xFF
  }

  return offset + byteLength
}

Buffer.prototype.writeInt8 = function writeInt8 (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 1, 0x7f, -0x80)
  if (!Buffer.TYPED_ARRAY_SUPPORT) value = Math.floor(value)
  if (value < 0) value = 0xff + value + 1
  this[offset] = (value & 0xff)
  return offset + 1
}

Buffer.prototype.writeInt16LE = function writeInt16LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0x7fff, -0x8000)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value & 0xff)
    this[offset + 1] = (value >>> 8)
  } else {
    objectWriteUInt16(this, value, offset, true)
  }
  return offset + 2
}

Buffer.prototype.writeInt16BE = function writeInt16BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 2, 0x7fff, -0x8000)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 8)
    this[offset + 1] = (value & 0xff)
  } else {
    objectWriteUInt16(this, value, offset, false)
  }
  return offset + 2
}

Buffer.prototype.writeInt32LE = function writeInt32LE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0x7fffffff, -0x80000000)
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value & 0xff)
    this[offset + 1] = (value >>> 8)
    this[offset + 2] = (value >>> 16)
    this[offset + 3] = (value >>> 24)
  } else {
    objectWriteUInt32(this, value, offset, true)
  }
  return offset + 4
}

Buffer.prototype.writeInt32BE = function writeInt32BE (value, offset, noAssert) {
  value = +value
  offset = offset | 0
  if (!noAssert) checkInt(this, value, offset, 4, 0x7fffffff, -0x80000000)
  if (value < 0) value = 0xffffffff + value + 1
  if (Buffer.TYPED_ARRAY_SUPPORT) {
    this[offset] = (value >>> 24)
    this[offset + 1] = (value >>> 16)
    this[offset + 2] = (value >>> 8)
    this[offset + 3] = (value & 0xff)
  } else {
    objectWriteUInt32(this, value, offset, false)
  }
  return offset + 4
}

function checkIEEE754 (buf, value, offset, ext, max, min) {
  if (offset + ext > buf.length) throw new RangeError('Index out of range')
  if (offset < 0) throw new RangeError('Index out of range')
}

function writeFloat (buf, value, offset, littleEndian, noAssert) {
  if (!noAssert) {
    checkIEEE754(buf, value, offset, 4, 3.4028234663852886e+38, -3.4028234663852886e+38)
  }
  ieee754.write(buf, value, offset, littleEndian, 23, 4)
  return offset + 4
}

Buffer.prototype.writeFloatLE = function writeFloatLE (value, offset, noAssert) {
  return writeFloat(this, value, offset, true, noAssert)
}

Buffer.prototype.writeFloatBE = function writeFloatBE (value, offset, noAssert) {
  return writeFloat(this, value, offset, false, noAssert)
}

function writeDouble (buf, value, offset, littleEndian, noAssert) {
  if (!noAssert) {
    checkIEEE754(buf, value, offset, 8, 1.7976931348623157E+308, -1.7976931348623157E+308)
  }
  ieee754.write(buf, value, offset, littleEndian, 52, 8)
  return offset + 8
}

Buffer.prototype.writeDoubleLE = function writeDoubleLE (value, offset, noAssert) {
  return writeDouble(this, value, offset, true, noAssert)
}

Buffer.prototype.writeDoubleBE = function writeDoubleBE (value, offset, noAssert) {
  return writeDouble(this, value, offset, false, noAssert)
}

// copy(targetBuffer, targetStart=0, sourceStart=0, sourceEnd=buffer.length)
Buffer.prototype.copy = function copy (target, targetStart, start, end) {
  if (!start) start = 0
  if (!end && end !== 0) end = this.length
  if (targetStart >= target.length) targetStart = target.length
  if (!targetStart) targetStart = 0
  if (end > 0 && end < start) end = start

  // Copy 0 bytes; we're done
  if (end === start) return 0
  if (target.length === 0 || this.length === 0) return 0

  // Fatal error conditions
  if (targetStart < 0) {
    throw new RangeError('targetStart out of bounds')
  }
  if (start < 0 || start >= this.length) throw new RangeError('sourceStart out of bounds')
  if (end < 0) throw new RangeError('sourceEnd out of bounds')

  // Are we oob?
  if (end > this.length) end = this.length
  if (target.length - targetStart < end - start) {
    end = target.length - targetStart + start
  }

  var len = end - start
  var i

  if (this === target && start < targetStart && targetStart < end) {
    // descending copy from end
    for (i = len - 1; i >= 0; --i) {
      target[i + targetStart] = this[i + start]
    }
  } else if (len < 1000 || !Buffer.TYPED_ARRAY_SUPPORT) {
    // ascending copy from start
    for (i = 0; i < len; ++i) {
      target[i + targetStart] = this[i + start]
    }
  } else {
    Uint8Array.prototype.set.call(
      target,
      this.subarray(start, start + len),
      targetStart
    )
  }

  return len
}

// Usage:
//    buffer.fill(number[, offset[, end]])
//    buffer.fill(buffer[, offset[, end]])
//    buffer.fill(string[, offset[, end]][, encoding])
Buffer.prototype.fill = function fill (val, start, end, encoding) {
  // Handle string cases:
  if (typeof val === 'string') {
    if (typeof start === 'string') {
      encoding = start
      start = 0
      end = this.length
    } else if (typeof end === 'string') {
      encoding = end
      end = this.length
    }
    if (val.length === 1) {
      var code = val.charCodeAt(0)
      if (code < 256) {
        val = code
      }
    }
    if (encoding !== undefined && typeof encoding !== 'string') {
      throw new TypeError('encoding must be a string')
    }
    if (typeof encoding === 'string' && !Buffer.isEncoding(encoding)) {
      throw new TypeError('Unknown encoding: ' + encoding)
    }
  } else if (typeof val === 'number') {
    val = val & 255
  }

  // Invalid ranges are not set to a default, so can range check early.
  if (start < 0 || this.length < start || this.length < end) {
    throw new RangeError('Out of range index')
  }

  if (end <= start) {
    return this
  }

  start = start >>> 0
  end = end === undefined ? this.length : end >>> 0

  if (!val) val = 0

  var i
  if (typeof val === 'number') {
    for (i = start; i < end; ++i) {
      this[i] = val
    }
  } else {
    var bytes = Buffer.isBuffer(val)
      ? val
      : utf8ToBytes(new Buffer(val, encoding).toString())
    var len = bytes.length
    for (i = 0; i < end - start; ++i) {
      this[i + start] = bytes[i % len]
    }
  }

  return this
}

// HELPER FUNCTIONS
// ================

var INVALID_BASE64_RE = /[^+\/0-9A-Za-z-_]/g

function base64clean (str) {
  // Node strips out invalid characters like \n and \t from the string, base64-js does not
  str = stringtrim(str).replace(INVALID_BASE64_RE, '')
  // Node converts strings with length < 2 to ''
  if (str.length < 2) return ''
  // Node allows for non-padded base64 strings (missing trailing ===), base64-js does not
  while (str.length % 4 !== 0) {
    str = str + '='
  }
  return str
}

function stringtrim (str) {
  if (str.trim) return str.trim()
  return str.replace(/^\s+|\s+$/g, '')
}

function toHex (n) {
  if (n < 16) return '0' + n.toString(16)
  return n.toString(16)
}

function utf8ToBytes (string, units) {
  units = units || Infinity
  var codePoint
  var length = string.length
  var leadSurrogate = null
  var bytes = []

  for (var i = 0; i < length; ++i) {
    codePoint = string.charCodeAt(i)

    // is surrogate component
    if (codePoint > 0xD7FF && codePoint < 0xE000) {
      // last char was a lead
      if (!leadSurrogate) {
        // no lead yet
        if (codePoint > 0xDBFF) {
          // unexpected trail
          if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
          continue
        } else if (i + 1 === length) {
          // unpaired lead
          if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
          continue
        }

        // valid lead
        leadSurrogate = codePoint

        continue
      }

      // 2 leads in a row
      if (codePoint < 0xDC00) {
        if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
        leadSurrogate = codePoint
        continue
      }

      // valid surrogate pair
      codePoint = (leadSurrogate - 0xD800 << 10 | codePoint - 0xDC00) + 0x10000
    } else if (leadSurrogate) {
      // valid bmp char, but last char was a lead
      if ((units -= 3) > -1) bytes.push(0xEF, 0xBF, 0xBD)
    }

    leadSurrogate = null

    // encode utf8
    if (codePoint < 0x80) {
      if ((units -= 1) < 0) break
      bytes.push(codePoint)
    } else if (codePoint < 0x800) {
      if ((units -= 2) < 0) break
      bytes.push(
        codePoint >> 0x6 | 0xC0,
        codePoint & 0x3F | 0x80
      )
    } else if (codePoint < 0x10000) {
      if ((units -= 3) < 0) break
      bytes.push(
        codePoint >> 0xC | 0xE0,
        codePoint >> 0x6 & 0x3F | 0x80,
        codePoint & 0x3F | 0x80
      )
    } else if (codePoint < 0x110000) {
      if ((units -= 4) < 0) break
      bytes.push(
        codePoint >> 0x12 | 0xF0,
        codePoint >> 0xC & 0x3F | 0x80,
        codePoint >> 0x6 & 0x3F | 0x80,
        codePoint & 0x3F | 0x80
      )
    } else {
      throw new Error('Invalid code point')
    }
  }

  return bytes
}

function asciiToBytes (str) {
  var byteArray = []
  for (var i = 0; i < str.length; ++i) {
    // Node's code seems to be doing this and not & 0x7F..
    byteArray.push(str.charCodeAt(i) & 0xFF)
  }
  return byteArray
}

function utf16leToBytes (str, units) {
  var c, hi, lo
  var byteArray = []
  for (var i = 0; i < str.length; ++i) {
    if ((units -= 2) < 0) break

    c = str.charCodeAt(i)
    hi = c >> 8
    lo = c % 256
    byteArray.push(lo)
    byteArray.push(hi)
  }

  return byteArray
}

function base64ToBytes (str) {
  return base64.toByteArray(base64clean(str))
}

function blitBuffer (src, dst, offset, length) {
  for (var i = 0; i < length; ++i) {
    if ((i + offset >= dst.length) || (i >= src.length)) break
    dst[i + offset] = src[i]
  }
  return i
}

function isnan (val) {
  return val !== val // eslint-disable-line no-self-compare
}

}).call(this,typeof global !== "undefined" ? global : typeof self !== "undefined" ? self : typeof window !== "undefined" ? window : {})
},{"base64-js":1,"ieee754":3,"isarray":4}],3:[function(require,module,exports){
exports.read = function (buffer, offset, isLE, mLen, nBytes) {
  var e, m
  var eLen = nBytes * 8 - mLen - 1
  var eMax = (1 << eLen) - 1
  var eBias = eMax >> 1
  var nBits = -7
  var i = isLE ? (nBytes - 1) : 0
  var d = isLE ? -1 : 1
  var s = buffer[offset + i]

  i += d

  e = s & ((1 << (-nBits)) - 1)
  s >>= (-nBits)
  nBits += eLen
  for (; nBits > 0; e = e * 256 + buffer[offset + i], i += d, nBits -= 8) {}

  m = e & ((1 << (-nBits)) - 1)
  e >>= (-nBits)
  nBits += mLen
  for (; nBits > 0; m = m * 256 + buffer[offset + i], i += d, nBits -= 8) {}

  if (e === 0) {
    e = 1 - eBias
  } else if (e === eMax) {
    return m ? NaN : ((s ? -1 : 1) * Infinity)
  } else {
    m = m + Math.pow(2, mLen)
    e = e - eBias
  }
  return (s ? -1 : 1) * m * Math.pow(2, e - mLen)
}

exports.write = function (buffer, value, offset, isLE, mLen, nBytes) {
  var e, m, c
  var eLen = nBytes * 8 - mLen - 1
  var eMax = (1 << eLen) - 1
  var eBias = eMax >> 1
  var rt = (mLen === 23 ? Math.pow(2, -24) - Math.pow(2, -77) : 0)
  var i = isLE ? 0 : (nBytes - 1)
  var d = isLE ? 1 : -1
  var s = value < 0 || (value === 0 && 1 / value < 0) ? 1 : 0

  value = Math.abs(value)

  if (isNaN(value) || value === Infinity) {
    m = isNaN(value) ? 1 : 0
    e = eMax
  } else {
    e = Math.floor(Math.log(value) / Math.LN2)
    if (value * (c = Math.pow(2, -e)) < 1) {
      e--
      c *= 2
    }
    if (e + eBias >= 1) {
      value += rt / c
    } else {
      value += rt * Math.pow(2, 1 - eBias)
    }
    if (value * c >= 2) {
      e++
      c /= 2
    }

    if (e + eBias >= eMax) {
      m = 0
      e = eMax
    } else if (e + eBias >= 1) {
      m = (value * c - 1) * Math.pow(2, mLen)
      e = e + eBias
    } else {
      m = value * Math.pow(2, eBias - 1) * Math.pow(2, mLen)
      e = 0
    }
  }

  for (; mLen >= 8; buffer[offset + i] = m & 0xff, i += d, m /= 256, mLen -= 8) {}

  e = (e << mLen) | m
  eLen += mLen
  for (; eLen > 0; buffer[offset + i] = e & 0xff, i += d, e /= 256, eLen -= 8) {}

  buffer[offset + i - d] |= s * 128
}

},{}],4:[function(require,module,exports){
var toString = {}.toString;

module.exports = Array.isArray || function (arr) {
  return toString.call(arr) == '[object Array]';
};

},{}],5:[function(require,module,exports){
var ENV = require('./common/env'),
  CONST = require('./common/const'),
  domUtils = require('./domUtils/domExtend'),
  amend = require('./amend/amend'),
  historyUtils = require('./common/historyUtils'),
  base64 = require('./common/base64'),
  wizStyle = require('./common/wizStyle'),
  rangeUtils = require('./rangeUtils/rangeExtend'),
  codeCore = require('./codeUtils/codeCore'),
  formatPainterCore = require('./formatPainterUtils/formatPainterCore'),
  tableCore = require('./tableUtils/tableCore'),
  todoCore = require('./todoUtils/todoCore'),
  linkUtils = require('./linkUtils/linkUtils'),
  imgUtils = require('./imgUtils/imgUtils'),
  nightModeUtils = require('./nightMode/nightModeUtils'),
  editor = require('./editor/base'),
  editorEvent = require('./editor/editorEvent'),
  commandExtend = require('./editor/commandExtend'),
  pasteUtils = require('./editor/pasteUtils');

var WizEditor = {
  version: '1.1',
  // clearWizDom: function(){},
  /**
   * 初始化 修订编辑
   * @param options
   * {
     *   document, //document
     *   lang,      //语言 JSON
     *   userInfo,  //用户数据 JSON
     *   usersData,  //kb内所有用户数据集合 Array[JSON]
     *   maxRedo,   //redo 最大堆栈数（默认100）
     *   callback {
     *      redo,  //history callback
     *   },
     *   clientType,  //客户端类型
     * }
   */
  init: function (options) {
    ENV.init('editor', options);
    ENV.dependency.files.init();

    editor.init();
    amend.initUser();
    amend.setUsersData();

    if (ENV.win.WizTemplate) {
      ENV.win.WizTemplate.init({
        document: ENV.doc,
        lang: ENV.options.lang,
        clientType: ENV.options.clientType
      });
    }
    return WizEditor;
  },
  /**
   * 启动编辑器
   */
  on: function (callback) {
    if (ENV.readonly === false) {
      return;
    }
    var scrollTopLast = ENV.doc.body.scrollTop;
    if (ENV.win.WizReader) {
      ENV.win.WizReader.off();
    }

    wizStyle.removeStyleByName(CONST.NAME.STYLE_FOR_LOAD);
    editor.on(_callback);

    function _callback() {
      if (ENV.readonly === true) {
        // 避免频繁切换 阅读、编辑模式
        return;
      }
      // console.log('wizEditor on callback');
      // console.log('isWin: ' + ENV.client.type.isWin + ', isWeb: ' + ENV.client.type.isWeb);
      if (ENV.win.WizTemplate) {
        ENV.win.WizTemplate.on(false);
      }
      if (ENV.options.autoFocus) {
        if (ENV.win.WizTemplate) {
          ENV.win.WizTemplate.focus();
        } else {
          WizEditor.focus();
        }
      }

      setTimeout(function () {
        if (!ENV.client.type.isPhone || !ENV.client.type.isPad) {
          if (ENV.client.type.isMac && ENV.options.noteType !== CONST.NOTE_TYPE.COMMON) {
            setTimeout(function () {
              ENV.doc.body.scrollTop = scrollTopLast;
            }, 100);
          } else {
            ENV.doc.body.scrollTop = scrollTopLast;
          }
        }

        if (typeof callback === 'function') {
          callback();
        }
      }, 0);
    }

    return WizEditor;
  },
  /**
   * 关闭编辑器
   */
  off: function (options, callback) {
    if (ENV.readonly === true) {
      return;
    }
    var scrollTopLast = ENV.doc.body.scrollTop;
    if (ENV.win.WizTemplate) {
      ENV.win.WizTemplate.off();
    }
    editor.off();
    wizStyle.removeStyleByName(CONST.NAME.STYLE_FOR_LOAD);

    if (ENV.win.WizReader) {
      ENV.win.WizReader.on(options, _callback);
    }
    return WizEditor;

    function _callback() {
      if (ENV.readonly === false) {
        // 避免频繁切换 阅读、编辑模式
        return;
      }
      // console.log('wizReader on callback');
      // console.log('isWin: ' + ENV.client.type.isWin + ', isWeb: ' + ENV.client.type.isWeb);
      if (!ENV.client.type.isPhone || !ENV.client.type.isPad) {
        ENV.doc.body.scrollTop = scrollTopLast;
      }

      if (typeof callback === 'function') {
        callback();
      }
    }
  },
  /**
   * 备份光标位置
   */
  backupCaret: function () {
    return rangeUtils.backupCaret();
  },
  execCommand: function () {
    historyUtils.saveSnap(false);
    return commandExtend.execCommand.apply(this, arguments);
  },
  /**
   * 在编辑时，查找指定文本
   * @param string
   * @param matchcase
   * @param searchBackward
   * @param loop
   * @returns {*}
   */
  find: function (str, matchcase, searchBackward, loop) {
    return editor.find(str, matchcase, searchBackward, loop);
  },
  /**
   * 让 body 获取焦点
   */
  focus: function () {
    domUtils.focus();
    return WizEditor;
  },
  /**
   * 获取 body 内正文，用于生成摘要
   * @returns {*}
   */
  getBodyText: function () {
    return domUtils.getBodyText();
  },
  /**
   * 获取当前页面源码
   * @param options
   * @returns {*}
   */
  getContentHtml: function (options) {
    // 只有当客户端获取笔记内容进行保存时，才做这些清理操作

    // 清理子节点时，必须放弃光标所在位置，否则会导致光标跳动
    var excludeList = [];
    var range = rangeUtils.getRange();
    var parent;
    if (range) {
      parent = domUtils.getParentRoot([range.startContainer, range.endContainer]);
      if (parent) {
        excludeList = [parent];
      }
    }
    domUtils.clearChild(ENV.doc.body, excludeList);

    amend.hideAmendInfo();
    todoCore.checkTodoStyle();

    return domUtils.getContentHtml(options);
  },
  insertDefaultStyle: function (onlyReplace, customCss) {
    wizStyle.insertDefaultStyle(onlyReplace, customCss);
    return WizEditor;
  },
  insertCustomStyle: function (id, customCss, isTemp) {
    wizStyle.insertCustomStyle(id, customCss, isTemp);
    return WizEditor;
  },
  /**
   * 在光标位置插入 base64 格式的html
   * @param b64Html
   * @param notInsertEmptyDiv
   */
  insertB64Html: function (b64Html, notInsertEmptyDiv) {
    editor.insertHtml(base64.decode(b64Html), notInsertEmptyDiv);
  },
  /**
   * 在光标位置插入 html
   * @param html
   * @param notInsertEmptyDiv
   */
  insertHtml: function (html, notInsertEmptyDiv) {
    editor.insertHtml(html, notInsertEmptyDiv);
  },
  /**
   * 判断编辑内容是否被修改
   * @returns {boolean}
   */
  isModified: function () {
    // var result = (!ENV.readonly && domUtils.getContentHtml() != editor.getOriginalHtml());
    // console.log('\n\n\n-------------------------------' + result + '--------------------------------------------------------------------------------------------\n\n\n');
    // if (result) {
    //     console.log(domUtils.getContentHtml());
    //     console.log('\n\n\n---------------------------------------------------------------------------------------------------------------------------\n\n\n');
    //     console.log(editor.getOriginalHtml());
    //     console.log('\n\n\n---------------------------------------------------------------------------------------------------------------------------\n\n\n');
    // }
    var result = ENV.readonly === false;
    if (!result) {
      return result;
    }
    var curHtml = domUtils.getContentHtml();
    var originalHtml = editor.getOriginalHtml();
    curHtml = domUtils.removeByTagFromHtml(curHtml, CONST.TAG.CODE_MIRROR);
    originalHtml = domUtils.removeByTagFromHtml(originalHtml, CONST.TAG.CODE_MIRROR);
    return ENV.readonly === false && curHtml != originalHtml;
  },
  /**
   * 修改光标选中文本的样式 和 属性
   * @param style (example:{'font-size':'16px', 'color':'red'})
   * @param attr
   */
  modifySelectionDom: function (style, attr) {
    editor.modifySelectionDom(style, attr);
  },
  paste: function (html, txt) {
    pasteUtils.pasteFromClient(html, txt);
  },
  pasteB64: function (b64Html, b64Txt) {
    WizEditor.paste(base64.decode(b64Html), base64.decode(b64Txt));
  },
  /**
   * 编辑器 redo
   */
  redo: function () {
    historyUtils.redo();
    return WizEditor;
  },
  removeFormat: function (removeAll, isRemoveColor, isRemoveAllStyles) {
    wizStyle.removeFormat(removeAll, isRemoveColor, isRemoveAllStyles);
    return WizEditor;
  },
  removeStyleById: function (id) {
    wizStyle.removeStyleById(id);
    return WizEditor;
  },
  replace: function (from, to, matchcase) {
    return editor.replace(from, to, matchcase);
  },
  replaceAll: function (from, to, matchcase) {
    editor.replaceAll(from, to, matchcase);
  },
  /**
   * 恢复已备份光标位置
   */
  restoreCaret: function () {
    return rangeUtils.restoreCaret();
  },
  /**
   * 编辑器 保存快照
   */
  saveSnap: function () {
    historyUtils.saveSnap(false);
    return WizEditor;
  },
  // /**
  //  * 专门针对 IOS 修正光标位置
  //  * @param data
  //  */
  // setWebViewSizeForFixScroll: function (data) {
  //   data = JSON.parse(base64.decode(data));
  //   ENV.options.ios.webViewHeight = data.height;
  //   ENV.options.ios.toolbarHeight = data.toolbarHeight;
  // },
  /**
   * 设置当前文档为 未修改状态
   */
  setUnModified: function () {
    editor.setOriginalHtml();
  },
  /**
   * 编辑器 undo
   */
  undo: function () {
    historyUtils.undo();
    return WizEditor;
  },
  ListenerType: editorEvent.TYPE,
  addListener: function (eName, fun) {
    editorEvent.addListener(eName, fun);
    return WizEditor;
  },
  removeListener: function (eName, fun) {
    editorEvent.removeListener(eName, fun);
    return WizEditor;
  },
  triggerListener: function (eName, params) {
    editorEvent.triggerListener(eName, params);
    return WizEditor;
  },
  startTrackEvent: function (eventName, id) {
    editorEvent.startTrackEvent(eventName, id);
  },
  stopTrackEvent: function (eventName, id) {
    editorEvent.stopTrackEvent(eventName, id);
  },
  amend: {
    /**
     * 开启 修订功能
     * @param status  // true：开启修订； false：关闭修订
     */
    on: function () {
      amend.start();

      return WizEditor;
    },
    /**
     * 关闭 修订功能
     */
    off: function () {
      //关闭 修订功能 需要同时开启 逆修订功能
      amend.startReverse();

      return WizEditor;
    },
    /**
     * 获取 笔记是否被进行过 修订编辑
     * @returns {boolean}
     */
    isEdited: function () {
      return amend.isAmendEdited();
    },
    /**
     * 获取 笔记当前 修订状态
     * @returns {boolean}
     */
    isEditing: function () {
      return amend.isAmendEditing();
    },
    /**
     * 判断当前光标位置是否处于修订标签内
     * @returns {boolean}
     */
    hasAmendSpanByCursor: function () {
      return amend.hasAmendSpanByCursor();
    },
    /**
     * 接受 修订内容， 清理所有修订的标签
     * @params options
     */
    accept: function (options) {
      amend.accept(initAmendAcceptOptions(options));
    },
    /**
     * 拒绝 修订内容， 恢复原内容
     * @param options
     */
    refuse: function (options) {
      amend.refuse(initAmendAcceptOptions(options));
    }
  },
  code: {
    insertCode: codeCore.insertCode
  },
  formatPainter: {
    on: function (keep) {
      return formatPainterCore.on(keep);
    },
    off: function () {
      formatPainterCore.off();
    }
  },
  img: {
    getAll: function (onlyLocal) {
      //为了保证客户端使用方便，转换为字符串
      return imgUtils.getAll(onlyLocal).join(',');
    },
    insertAsAttachment: function (guid, imgPath) {
      editor.insertDom(imgUtils.makeAttachmentDom(guid, imgPath), true);
    },
    insertByPath: function (imgPath) {
      editor.insertDom(imgUtils.makeDomByPath(imgPath), true);
    },
    removeCur: function () {
      var selector = 'img[' + CONST.ATTR.IMG_EDITING + ']';
      imgUtils.remove(selector);
    },
    replaceCur: function (targetSrc) {
      var selector = 'img[' + CONST.ATTR.IMG_EDITING + ']';
      imgUtils.replaceSrc(selector, targetSrc);
    }
  },
  link: {
    /**
     * 开启 自动设置 超链接功能
     */
    on: function () {
      linkUtils.on();
    },
    /**
     * 关闭 自动设置 超链接功能
     */
    off: function () {
      linkUtils.off();
    },
    /**
     * 获取光标位置的超链接 URL
     * @returns {*}
     */
    getCurrentLink: function () {
      return linkUtils.getCurrentLink();
    },
    /**
     * 移除选中的 <a> 标签的超链接
     */
    removeSelectedLink: function () {
      linkUtils.removeSelectedLink();
    },
    /**
     * 设置当前选中区域超链接，或在光标处添加超链接
     * @param url
     */
    setCurrentLink: function (url) {
      linkUtils.setCurrentLink(url);
    }
  },
  range: {
    moveToPoint: function(x, y) {
      rangeUtils.moveToPoint(x, y);
    }
  },
  table: {
    canCreateTable: tableCore.canCreateTable,
    clearCellValue: tableCore.clearCellValue,
    deleteCols: tableCore.deleteCols,
    deleteRows: tableCore.deleteRows,
    deleteTable: tableCore.deleteTable,
    distributeCols: tableCore.distributeCols,
    insertCol: tableCore.insertCol,
    insertRow: tableCore.insertRow,
    insertTable: tableCore.insertTable,
    merge: tableCore.merge,
    setCellAlign: tableCore.setCellAlign,
    setCellBg: tableCore.setCellBg,
    split: tableCore.split
  },
  todo: {
    setTodo: todoCore.setTodo,
    setTodoInfo: todoCore.setTodoInfo
  },
  nightMode: {
    on: function (color, bgColor, brightness) {
      nightModeUtils.on(color, bgColor, brightness);
    },
    off: function () {
      nightModeUtils.off();
    }
  },
  utils: {
    clearStyleFromHtml: function (html, whiteList) {
      return domUtils.clearStyleFromHtml(html, whiteList);
    }
  }
};

function initAmendAcceptOptions(options) {
  if (!options) {
    options = {
      dom: null,
      cursor: false,
      total: true
    };
  } else {
    options.total = !!options.total;
    options.dom = options.dom;
    options.cursor = !!options.cursor;
  }
  return options;
}

window.WizEditor = WizEditor;

module.exports = WizEditor;
},{"./amend/amend":7,"./codeUtils/codeCore":14,"./common/base64":17,"./common/const":18,"./common/env":20,"./common/historyUtils":21,"./common/wizStyle":25,"./domUtils/domExtend":29,"./editor/base":32,"./editor/commandExtend":33,"./editor/editorEvent":34,"./editor/pasteUtils":35,"./formatPainterUtils/formatPainterCore":37,"./imgUtils/imgUtils":42,"./linkUtils/linkUtils":43,"./nightMode/nightModeUtils":47,"./rangeUtils/rangeExtend":49,"./tableUtils/tableCore":52,"./todoUtils/todoCore":56}],6:[function(require,module,exports){
var ENV = require('./common/env'),
    CONST = require('./common/const'),
    utils = require('./common/utils'),
    base64 = require('./common/base64'),
    wizStyle = require('./common/wizStyle'),
    imgUtils = require('./imgUtils/imgUtils'),
    nightModeUtils = require('./nightMode/nightModeUtils'),
    amendInfo = require('./amend/amendInfo'),
    amendUser = require('./amend/amendUser'),
    todoCore = require('./todoUtils/todoCore'),
    highlight = require('./highlightUtils/highlightUtils'),
    reader = require('./reader/base'),
    markdownRender = require('./markdown/markdownRender');

var WizReader = {
    /**
     * 初始化 修订编辑
     * @param options
     * {
     *   document, //document
     *   lang,      //语言 JSON
     *   userInfo,  //用户数据 JSON
     *   usersData,  //kb内所有用户数据集合 Array[JSON]
     *   clientType,  //客户端类型,
     *   noAmend, //Boolean 是否显示 修订信息
     *   noteType // 'common' 'markdown' 'mathjax' 笔记类型
     * }
     */
    init: function (options) {
        ENV.init('reader', options);
        ENV.dependency.files.init();

        reader.init();
        markdownRender.init();

        if (!ENV.options.noAmend) {
            amendUser.initUser(ENV.options.userInfo);
            amendUser.setUsersData(ENV.options.usersData);
        }

        if (ENV.win.WizTemplate) {
            ENV.win.WizTemplate.init({
                document: ENV.doc,
                lang: ENV.options.lang,
                clientType: ENV.options.clientType
            });
        }

        return WizReader;
    },
    /**
     *
     * @param options
     * {
     *   noteType 笔记类型
     * }
     */
    on: function (options, callback) {
        if (options && options.noteType) {
            ENV.options.noteType = options.noteType;
        }
        ENV.options.pc.pluginModified = false;
        reader.on();

        WizReader.amendInfo.on();
        if (ENV.win.WizTemplate) {
            ENV.win.WizTemplate.on(true);
        }

        if (ENV.options.noteType === CONST.NOTE_TYPE.MARKDOWN) {
            WizReader.markdown(_callback);
        } else if (ENV.options.noteType === CONST.NOTE_TYPE.MATHJAX) {
            WizReader.mathJax(_callback);
        } else {
            setTimeout(function() {
                _callback();
            }, 0);
        }
        return WizReader;

        function _callback() {
            if (ENV.readonly === false) {
                // 避免频繁切换 阅读、编辑模式
                return;
            }
            reader.afterRender(function() {
                if (typeof callback === 'function') {
                    callback();
                }
            });
        }
    },
    off: function () {
        highlight.off();
        WizReader.amendInfo.off();

        if (ENV.win.WizTemplate) {
            ENV.win.WizTemplate.off();
        }

        reader.off();

        return WizReader;
    },
    closeDocument: function () {
        return todoCore.closeDocument();
    },
    getWordCount: function () {
        var text = ENV.doc.body.innerText ? ENV.doc.body.innerText : '';
        var count = utils.getWordCount(text);
        return JSON.stringify(count);
    },
    highlight: {
        next: function () {
            highlight.next();
        },
        on: function (keyList, focusFirst) {
            return highlight.on(keyList, focusFirst);
        },
        off: function () {
            highlight.off();
        },
        previous: function () {
            highlight.previous();
        },
    },
    insertDefaultStyle: function (onlyReplace, customCss) {
        wizStyle.insertDefaultStyle(onlyReplace, customCss);
        return WizReader;
    },
    insertCustomStyle: function (id, customCss, isTemp) {
        wizStyle.insertCustomStyle(id, customCss, isTemp);
        return WizReader;
    },
    removeStyleById: function (id) {
        wizStyle.removeStyleById(id);
        return WizReader;
    },
    markdown: function (callback, timeout) {
        timeout = timeout ? timeout : ENV.options.timeout.markdown;
        callback = callback || ENV.options.callback.markdown;
        var hasCalled = false,
            cb = function () {
                if (callback && /^function$/i.test(typeof callback) && !hasCalled) {
                    callback();
                    hasCalled = true;
                }
            };
        markdownRender.markdown({
            markdown: function (isMathjax) {
                if (!isMathjax) {
                    cb();
                } else {
                    setTimeout(cb, timeout);
                }
            },
            mathJax: function () {
                cb();
            }
        });
    },
    mathJax: function (callback, timeout) {
        timeout = timeout ? timeout : ENV.options.timeout.mathJax;
        callback = callback || ENV.options.callback.mathJax;
        var hasCalled = false,
            cb = function () {
                if (callback && !hasCalled) {
                    callback();
                    hasCalled = true;
                }
            };

        setTimeout(cb, timeout);
        markdownRender.mathJax(function () {
            cb();
        });
    },
    setPluginModify: function (flag) {
        if (ENV.options.pc.pluginModified) {
            return ENV.options.pc.pluginModified;
        }

        // markdown/mathjax 笔记不允许 plugin 修改状态
        if (ENV.options.noteType === CONST.NOTE_TYPE.COMMON) {
            ENV.options.pc.pluginModified = !!flag;
        }

        return ENV.options.pc.pluginModified
    },
    amendInfo: {
        on: function () {
            if (ENV.options.noAmend) {
                return WizReader;
            }
            amendInfo.init({
                readonly: true
            }, {
                onAccept: null,
                onRefuse: null
            });

            return WizReader;
        },
        off: function () {
            amendInfo.remove();

            return WizReader;
        }
    },
    img: {
        getAll: function (onlyLocal) {
            //为了保证客户端使用方便，转换为字符串
            return imgUtils.getAll(onlyLocal).join(',');
        }
    },
    todo: {
        setTodoInfo: todoCore.setTodoInfo,
        onCheckDocLock: todoCore.onCheckDocLock
    },
    nightMode: {
        on: function (color, bgColor, brightness) {
            nightModeUtils.on(color, bgColor, brightness);
        },
        off: function () {
            nightModeUtils.off();
        }
    }
};

window.WizReader = WizReader;

module.exports = WizReader;
},{"./amend/amendInfo":8,"./amend/amendUser":9,"./common/base64":17,"./common/const":18,"./common/env":20,"./common/utils":24,"./common/wizStyle":25,"./highlightUtils/highlightUtils":38,"./imgUtils/imgUtils":42,"./markdown/markdownRender":46,"./nightMode/nightModeUtils":47,"./reader/base":50,"./todoUtils/todoCore":56}],7:[function(require,module,exports){
/**
 * 修订功能 专用工具包
 */

var ENV = require('../common/env'),
    CONST = require('../common/const'),
    Lang = require('../common/lang'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    LANG = Lang.getLang(),
    amendUtils = require('./amendUtils/amendExtend'),
    amendUser = require('./amendUser'),
    amendInfo = require('./amendInfo'),
    blockUtils = require('../blockUtils/blockUtils'),
    domUtils = require('../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    codeCore = require('../codeUtils/codeCore'),
    tableCore = require('../tableUtils/tableCore'),
    tableUtils = require('../tableUtils/tableUtils'),
    todoCore = require('../todoUtils/todoCore');

//为 domUtils 打补丁
(function () {
    var modifyNodeStyle = domUtils.modifyNodeStyle;
    //针对 修订特殊处理 image
    domUtils.modifyNodeStyle = function (item, style, attr, isLast) {
        var p;
        if (item.nodeType == 1 &&
            attr && attr[CONST.ATTR.SPAN_DELETE] && domUtils.isTag(item, 'img')) {
            amendUtils.deleteImg(item, amendUser.getCurUser());
            return item;

        } else if (item.nodeType == 1 &&
            attr && attr[CONST.ATTR.SPAN_DELETE] &&
            domUtils.isEmptyDom(item)) {
            //TODO 需要提取 判断br 的方法
            // 只能删除 被父节点内单独存在的 br
            p = item.parentNode;
            p.removeChild(item);
            domUtils.removeEmptyParent(p);
            return item;
        } else if (item.nodeType == 1 &&
            attr && attr[CONST.ATTR.SPAN_DELETE] && domUtils.isSelfClosingTag(item)) {
            return item;
        } else if (attr && attr[CONST.ATTR.SPAN_DELETE] && amendUtils.getWizDeleteParent(item)) {
            return item;
        } else {
            return modifyNodeStyle(item, style, attr, isLast);
        }
    };
    var addDomForGetDomList = domUtils.addDomForGetDomList;
    //忽略 在修订模式下 已经删除的内容
    domUtils.addDomForGetDomList = function (main, sub) {
        //忽略 在修订模式下 已经删除的内容
        if (amendUtils.isWizDelete(sub) ||
            //td tr 之间不能添加 span!!
            (sub.nodeType == 3 && !domUtils.getParentByTagName(sub, ['td', 'th'], false, null) && domUtils.getParentByTagName(sub, 'table', false, null))) {
            return;
        }
        addDomForGetDomList(main, sub);
    };
})();

var isAmendEditing = false;
var amend = {
    initUser: function () {
        amendUser.initUser(ENV.options.userInfo);
    },
    setUsersData: function () {
        amendUser.setUsersData(ENV.options.usersData);
    },
    /**
     * 开启 修订功能
     */
    start: function () {
        isAmendEditing = true;
        amend.stopReverse();
        amendEvent.bind();
        amend.startAmendInfo();
        ENV.event.add(CONST.EVENT.BEFORE_SAVESNAP, amendEvent.onBeforeSaveSnap);
        ENV.event.add(CONST.EVENT.AFTER_RESTORE_HISTORY, amendEvent.onAfterRestoreHistory);
    },
    /**
     * 关闭 修订功能
     */
    stop: function () {
        isAmendEditing = false;
        amendEvent.unbind();
        amendInfo.remove();
        if (!amend.isAmendEdited()) {
            //删除 所有修订者 信息
            //amendUser.removeAllUserInfo();
        }
        ENV.event.remove(CONST.EVENT.BEFORE_SAVESNAP, amendEvent.onBeforeSaveSnap);
        ENV.event.remove(CONST.EVENT.AFTER_RESTORE_HISTORY, amendEvent.onAfterRestoreHistory);
    },
    /**
     * 开启 反转修订功能
     */
    startReverse: function () {
        amend.stop();

        amendEvent.bindReverse();
        amend.startAmendInfo();
    },
    /**
     * 关闭 反转修订功能
     */
    stopReverse: function () {
        amendEvent.unbindReverse();
        amendInfo.remove();
        if (!amend.isAmendEdited()) {
            //删除 所有修订者 信息
            //amendUser.removeAllUserInfo();
        }
    },
    /**
     * 开启显示 修订信息
     */
    startAmendInfo: function (options) {
        amendInfo.init(options, {
            onAccept: amendEvent.onAccept,
            onRefuse: amendEvent.onRefuse
        });
    },
    /**
     * 关闭显示 修订信息
     */
    stopAmendInfo: function () {
        amendInfo.remove();
    },
    /**
     * 隐藏显示 修订信息（主要用于保存笔记前处理）
     */
    hideAmendInfo: function () {
        amendInfo.hide(true);
    },
    /**
     * 判断笔记是否存在 被修订的痕迹
     * @returns boolean
     */
    isAmendEdited: function () {
        return amendUtils.isAmendEdited();
    },
    isAmendEditing: function () {
        return isAmendEditing;
    },
    hasAmendSpanByCursor: function () {
        var amendDoms = amendUtils.getAmendDoms({
            selection: true,
            selectAll: false
        });

        return amendDoms.insertList.length > 0 ||
            amendDoms.deleteList.length > 0 ||
            amendDoms.deletedInsertList.length > 0;
    },
    /**
     * 接受 修订内容
     * @param target
     */
    accept: function (target) {
        var sel = ENV.doc.getSelection(),
            options = {}, amendDoms;

        if (target.total) {
            options.selection = true;
            options.selectAll = true;

        } else if (target.dom && !target.isSelection) {
            options.domList = amendUtils.getSameTimeStampDom(target.dom);
            options.selection = false;

        } else {
            //TODO 无光标焦点时，跳转到下一个修订内容
            if (sel.rangeCount === 0) {
                return;
            }

            options.selection = true;
            options.selectAll = false;
        }

        //先保存 内容快照，便于 undo
        historyUtils.saveSnap(false);

        if (options.selection && !options.selectAll) {
            amendDoms = amendUtils.getSelectedAmendDoms();
        } else {
            amendDoms = amendUtils.getAmendDoms(options);
        }

        if (amendDoms) {
            amendUtils.splitSelectedAmendDoms(amendDoms);

            //删除 已删除的
            amendUtils.wizAmendDelete(amendDoms.deleteList);
            amendUtils.wizAmendDelete(amendDoms.deletedInsertList);
            //保留 新添加的
            amendUtils.wizAmendSave(amendDoms.insertList);
        }

        //合并文档， 清除冗余html
        domUtils.clearChild(ENV.doc.body, []);
    },
    /**
     *  拒绝 修订内容
     *  @param target
     */
    refuse: function (target) {
        var sel = ENV.doc.getSelection(),
            options = {}, amendDoms;

        if (target.total) {
            options.selection = true;
            options.selectAll = true;

        } else if (target.dom && !target.isSelection) {
            options.domList = amendUtils.getSameTimeStampDom(target.dom);
            options.selection = false;
        } else {
            //TODO 无光标焦点时，跳转到下一个修订内容
            if (sel.rangeCount === 0) {
                return;
            }

            options.selection = true;
            options.selectAll = false;
        }

        //先保存 内容快照，便于 undo
        historyUtils.saveSnap(false);

        if (options.selection && !options.selectAll) {
            amendDoms = amendUtils.getSelectedAmendDoms();
        } else {
            amendDoms = amendUtils.getAmendDoms(options);
        }

        if (amendDoms) {
            amendUtils.splitSelectedAmendDoms(amendDoms);

            //对于 用户B 删除了用户A 新增的内容，只有单独选中该 dom 拒绝修订时， 才还原为 用户A 新增的内容，
            //否则拒绝时，一律当作 用户A 新增的内容进行删除操作
            var saveDeletedInsert = amendDoms.deletedInsertList.length > 0 &&
                amendDoms.deleteList.length == 0 &&
                amendDoms.insertList.length == 0;

            //保留 已删除的
            amendUtils.wizAmendSave(amendDoms.deleteList);
            if (saveDeletedInsert) {
                amendUtils.wizAmendSave(amendDoms.deletedInsertList);
            }
            //删除 新添加的
            amendUtils.wizAmendDelete(amendDoms.insertList);
            if (!saveDeletedInsert) {
                amendUtils.wizAmendDelete(amendDoms.deletedInsertList);
            }
        }

        //合并文档， 清除冗余html
        domUtils.clearChild(ENV.doc.body, []);
    },
    /**
     * 参考 amendUtils.splitAmendDomByRange
     * @param fixed
     */
    splitAmendDomByRange: function (fixed) {
        return amendUtils.splitAmendDomByRange(fixed);
    },
    /**
     * 复制时 根据 fragment 过滤修订内容
     * @param fragment
     */
    fragmentFilter: function (fragment) {
        var delDom, i, delDomItem;

        if (!fragment) {
            return false;
        }

        delDom = fragment.querySelectorAll('span[' + CONST.ATTR.SPAN_DELETE + '="' + amendUser.getCurUser().hash + '"]');
        for (i = delDom.length - 1; i >= 0; i--) {
            delDomItem = delDom[i];
            domUtils.remove(delDomItem);
        }
    },
    readyForPaste: function () {
        var sel = ENV.doc.getSelection(), range,
            endDomBak, endDom, endOffset,
            id, newDom,
            nSpanStart, nSpanContent, nSpanEnd, nSpanNext,
            nA, p, tmpSplit, splitInsert, amendImg, isTd;

        //无光标时， 不操作任何内容
        if (sel.rangeCount === 0) {
            return;
        }

        if (!sel.isCollapsed) {
            range = sel.getRangeAt(0);
            endDomBak = domUtils.getParentByTagName(range.endContainer, ['td', 'th'], true, null);
            amendUtils.removeSelection(amendUser.getCurUser());
            amendUtils.removeUserDel(null, amendUser.getCurUser());
        }

        range = sel.getRangeAt(0);
        endDom = range.endContainer;
        endOffset = range.endOffset;

        if (domUtils.isTag(endDom, ['td', 'th']) && endOffset === 0 && endDomBak !== endDom) {
            //清理 用户已删除内容时，可能会导致 光标进入到下一个 td 内，所以必须修正
            endDom = endDomBak;
            endOffset = domUtils.getEndOffset(endDom);
        }

        splitInsert = amendUtils.splitInsertDom(endDom, endOffset, true, amendUser.getCurUser());

        id = (new Date()).valueOf();
        newDom = amendUtils.createDomForPaste(id);
        nSpanStart = newDom.start;
        nSpanContent = newDom.content;
        nSpanEnd = newDom.end;
        amendImg = amendUtils.getWizAmendImgParent(endDom);

        if (splitInsert.split) {
            //如果在 用户新增的 span 内操作， 则在span 拆分后，添加到 两个 span 之间
            if (endDom.nodeType == 3) {
                endDom = endDom.parentNode;
            }
            domUtils.before([nSpanStart, nSpanContent, nSpanEnd], endDom, endOffset > 0);
        } else if (amendImg) {
            domUtils.after([nSpanStart, nSpanContent, nSpanEnd], amendImg);
        } else if (endDom.nodeType == 1) {
            // endDom nodeType == 1 时， 光标应该是在 childNodes[endOffset] 元素的前面
            isTd = false;
            if (domUtils.isTag(endDom, ['td', 'th'])) {
                //如果 target 是 td 则必须在 td内建立 span，避免插入到 td 后面
                if (domUtils.isEmptyDom(endDom)) {
                    endDom.innerHTML = '';
                    endDom.appendChild(domUtils.createSpan());
                }
                isTd = true;
            }

            if (endOffset < endDom.childNodes.length) {
                domUtils.before([nSpanStart, nSpanContent, nSpanEnd], endDom.childNodes[endOffset]);
            } else if (isTd) {
                endDom.appendChild(nSpanStart);
                endDom.appendChild(nSpanContent);
                endDom.appendChild(nSpanEnd);
            } else {
                domUtils.after([nSpanStart, nSpanContent, nSpanEnd], endDom);
            }

        } else if (endDom.nodeType == 3) {
            if (amendUtils.splitDeletedDom(endDom, endOffset)) {
                domUtils.after([nSpanStart, nSpanContent, nSpanEnd], endDom.parentNode);

            } else if (endOffset < endDom.nodeValue.length) {
                tmpSplit = ENV.doc.createTextNode(endDom.nodeValue.substr(endOffset));
                endDom.nodeValue = endDom.nodeValue.substr(0, endOffset);
                domUtils.after([nSpanStart, nSpanContent, nSpanEnd, tmpSplit], endDom);

            } else {
                nA = domUtils.getParentByTagName(endDom, 'a', true, null);
                nSpanNext = endDom.nextSibling;
                if (nA) {
                    //光标在 <A> 标签结尾的时候，一定要让光标进入 <A> 下一个Dom
                    domUtils.after([nSpanStart, nSpanContent, nSpanEnd], nA);
                } else if (nSpanNext) {
                    domUtils.before([nSpanStart, nSpanContent, nSpanEnd], nSpanNext);
                } else {
                    p = endDom.parentNode;
                    p.insertBefore(nSpanStart, null);
                    p.insertBefore(nSpanContent, null);
                    p.insertBefore(nSpanEnd, null);
                }
            }
        }

        //不能使用 selectAllChildren ，否则 输入 空格时 浏览器会自动复制前一个 span 的所有样式
//        sel.selectAllChildren(nSpanStart);
        rangeUtils.setRange(nSpanContent.childNodes[0], 0, nSpanContent.childNodes[0], 1);

        setTimeout(function () {
            //有时候 nSpanEnd 的 DOM 在 粘贴操作后会自动变成新的 DOM 导致处理异常，
            //所以必须重新获取 nSpanEnd
            nSpanEnd = ENV.doc.querySelector('span[' +
                CONST.ATTR.SPAN_PASTE_TYPE + '="' + CONST.TYPE.PASTE.END + '"][' +
                CONST.ATTR.SPAN_PASTE_ID + '="' + nSpanEnd.getAttribute(CONST.ATTR.SPAN_PASTE_ID) + '"]');
            amend.fixPaste(nSpanStart, nSpanEnd, amendUser.getCurUser());
        }, 200);
    },
    fixPaste: function (start, end, user) {
        amendUtils.modifyDomForPaste(start, end, user);
    }
};

/**
 * 修订操作的 事件处理
 */
var amendEvent = {
    /**
     * 初始化时，绑定修订相关的必要事件
     */
    bind: function () {
        amendEvent.unbind();
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, amendEvent.onKeyDown);
        ENV.event.add(CONST.EVENT.ON_COMPOSITION_START, amendEvent.onCompositionStart);
        ENV.event.add(CONST.EVENT.ON_COMPOSITION_END, amendEvent.onCompositionEnd);

        if (!(ENV.client.type.isIOS || ENV.client.type.isAndroid)) {
            ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, amendEvent.onMouseDown);
            ENV.event.add(CONST.EVENT.ON_MOUSE_UP, amendEvent.onMouseUp);
        } else {
            ENV.event.add(CONST.EVENT.ON_TOUCH_START, amendEvent.onTouchStart);
        }
        ENV.event.add(CONST.EVENT.ON_DRAG_START, amendEvent.onDragDrop);
        ENV.event.add(CONST.EVENT.ON_DRAG_ENTER, amendEvent.onDragDrop);
        ENV.event.add(CONST.EVENT.ON_DROP, amendEvent.onDragDrop);
    },
    /**
     * 解绑修订相关的必要事件
     */
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, amendEvent.onKeyDown);
        ENV.event.remove(CONST.EVENT.ON_COMPOSITION_START, amendEvent.onCompositionStart);
        ENV.event.remove(CONST.EVENT.ON_COMPOSITION_END, amendEvent.onCompositionEnd);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, amendEvent.onMouseDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_UP, amendEvent.onMouseUp);
        ENV.event.remove(CONST.EVENT.ON_TOUCH_START, amendEvent.onTouchStart);

        ENV.event.remove(CONST.EVENT.ON_DRAG_START, amendEvent.onDragDrop);
        ENV.event.remove(CONST.EVENT.ON_DRAG_ENTER, amendEvent.onDragDrop);
        ENV.event.remove(CONST.EVENT.ON_DROP, amendEvent.onDragDrop);
    },
    /**
     * 绑定反转修订相关的必要事件
     */
    bindReverse: function () {
        amendEvent.unbindReverse();
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, amendEvent.onKeyDown);
        ENV.event.add(CONST.EVENT.ON_COMPOSITION_START, amendEvent.onCompositionStart);
        ENV.event.add(CONST.EVENT.ON_COMPOSITION_END, amendEvent.onCompositionEnd);
        if (!ENV.client.type.isIOS && ENV.client.type.isAndroid) {
            ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, amendEvent.onMouseDown);
            ENV.event.add(CONST.EVENT.ON_MOUSE_UP, amendEvent.onMouseUp);
        }
    },
    /**
     * 解绑反转修订相关的必要事件
     */
    unbindReverse: function () {
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, amendEvent.onKeyDown);
        ENV.event.remove(CONST.EVENT.ON_COMPOSITION_START, amendEvent.onCompositionStart);
        ENV.event.remove(CONST.EVENT.ON_COMPOSITION_END, amendEvent.onCompositionEnd);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, amendEvent.onMouseDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_UP, amendEvent.onMouseUp);
    },
    /**
     * 点击 修订信息图层的 接受修订按钮 回调
     * @param target
     */
    onAccept: function (target) {
        amend.accept(target);
    },
    /**
     * 点击 修订信息图层的 拒绝修订按钮 回调
     * @param target
     */
    onRefuse: function (target) {
        amend.refuse(target);
    },
    /**
     * history 控件  beforeSaveSnap 保存快照之前的回调，用于在保存快照前执行必要操作
     */
    onBeforeSaveSnap: function () {
        //隐藏 修订信息浮动图层，避免 undo 保存多余的图层数据
        amendInfo.hide(true);
    },
    /**
     * history 控件  afterRestoreHistory 保存快照之后的回调，用于在保存快照后执行必要操作
     */
    onAfterRestoreHistory: function () {
        //重新设置 amendInfo 的图层对象
        amend.startAmendInfo();
    },
    /**
     * 中文输入开始
     */
    onCompositionStart: function () {
//            console.log('start....');
        CONST.COMPOSITION_START = true;
    },
    /**
     * 中文输入结束
     */
    onCompositionEnd: function () {
//            console.log('end....');
        CONST.COMPOSITION_START = false;
        //必须要延迟处理， 否则输入中文后按下 ESC 执行取消操作，触发此事件时，页面上还存在输入的中文拼音
        setTimeout(function () {
            historyUtils.saveSnap(true);
        }, 0);

    },
    /**
     * 拖拽 文件 或 文本
     * @param e
     */
    onDragDrop: function (e) {
        //修订编辑时 禁用 拖拽操作，否则无法控制输入的内容
        utils.stopEvent(e);
        return false;
    },
    /**
     * 按下键盘
     * @param e
     */
    onKeyDown: function (e) {
        var keyCode = e.keyCode || e.which;
        var sel = ENV.doc.getSelection();
//            console.info(e);

        if (keyCode === 8 || keyCode === 46) {
            //只有删除时才清理 特殊字符，避免 输入内容时 清空特殊字符导致光标跑到 span 外
            rangeUtils.clearFillCharByCollapsed();
        }

        if (!codeCore.onKeyDown(e) ||
            !tableCore.onKeyDown(e) ||
            !todoCore.onKeyDown(e)) {
            return;
        }

        //无光标时，或输入法开始后 不操作任何内容
        if (sel.rangeCount === 0 || CONST.COMPOSITION_START) {
            return;
        }

        if (amend.isAmendEditing()) {
            amendEvent.onKeyDownAmend(e);
        } else {
            amendEvent.onKeyDownReverse(e);
        }
    },
    /**
     * 按下键盘 for Amend
     * @param e
     */
    onKeyDownAmend: function (e) {
        var keyCode = e.keyCode || e.which;
        // console.info(e);
        var sel = ENV.doc.getSelection(), range,
            endDom, endOffset,
            nSpan, nSpanNext, nA, tmpA, tmpNext,
            tmpSplitStr, tmpSplit, tmpParentRoot;

        /**
         * Backspace
         */
        if (keyCode === 8) {
            historyUtils.saveSnap(false);

            if (!sel.isCollapsed) {
                amendUtils.removeSelection(amendUser.getCurUser());
                amendUtils.removeUserDel(null, amendUser.getCurUser());
                sel.collapseToStart();
            } else {
//                  console.log(endDom.nodeValue);
                rangeUtils.selectCharIncludeFillChar(true);
                amendUtils.removeSelection(amendUser.getCurUser());
                tmpParentRoot = rangeUtils.getRangeParentRoot();
                sel.collapseToStart();

                // 屏蔽无用代码
                // range = sel.getRangeAt(0);
                // endDom = range.startContainer;
                // if (endDom.nodeType === 3) {
                //     endDom = endDom.parentNode;
                // }

                //保证光标移动正确， isCollapsed 的时候，必须先移动光标再做删除操作
                amendUtils.removeUserDel(tmpParentRoot, amendUser.getCurUser());

                sel.collapseToStart();
            }
            rangeUtils.caretFocus();
            utils.stopEvent(e);
            return;
        }
        /**
         * Delete
         */
        if (keyCode === 46) {
            historyUtils.saveSnap(false);

            if (sel.isCollapsed) {
                rangeUtils.selectCharIncludeFillChar(false);
            }
            amendUtils.removeSelection(amendUser.getCurUser());
            amendUtils.removeUserDel(null, amendUser.getCurUser());
            sel.collapseToEnd();

            rangeUtils.caretFocus();
            utils.stopEvent(e);
            return;
        }

        /**
         * 其他功能键一概忽略
         */
        if (utils.checkNonTxtKey(e)) {
            return;
        }

        /**
         * 普通字符
         */

        /**
         * 先执行 execCommand， 后操作 range ， 执行 execCommand 之后， endDom 会发生改变
         * 先操作 range 后执行 execCommand， 无法控制当前选中的 endDom，一旦修改里面内容， execCommand 执行的结果就出现异常
         * 使用 自己的方法直接操作 dom，放弃 execCommand
         */
        var splitInsert, amendImg;
        historyUtils.saveSnap(false);
        if (!sel.isCollapsed) {
            amendUtils.removeSelection(amendUser.getCurUser());
            amendUtils.removeUserDel(null, amendUser.getCurUser());
        }
        range = sel.getRangeAt(0);
        endDom = range.endContainer;
        endOffset = range.endOffset;

        /**
         * Enter
         */
        if (keyCode === 13) {
            var delDom = amendUtils.getWizDeleteParent(endDom),
                insertDom = amendUtils.getWizInsertParent(endDom),
                isImg = !!insertDom ? amendUtils.getWizAmendImgParent(endDom) : false,
                aDom = delDom || insertDom;
            if (aDom && aDom.childNodes.length === 1 &&
                (!domUtils.isUsableTextNode(aDom.childNodes[0]) || (aDom.childNodes[0].nodeType === 1 && domUtils.isTag(aDom.childNodes[0], 'br')))) {
                //如果按下 Enter 键 时， 光标处于空白的 wizspan 标签内时，立刻删除该 span，避免span 的样式被 浏览器默认转换为 font 标签
                (function () {
                    var p = aDom.parentNode, b = ENV.doc.createElement('br');
                    p.insertBefore(b, aDom);
                    p.removeChild(aDom);
                    rangeUtils.setRange(b, 1, b, 1);
                })();
            } else if (insertDom && isImg) {
                //按下 Enter 键时，如果处于 IMG SPAN 区域内， 则直接在该区域最后添加span，避免 浏览器默认继承 样式
                (function () {
                    var s = domUtils.createSpan();
                    s.innerHTML = CONST.FILL_CHAR;
                    domUtils.after(s, insertDom);
                    rangeUtils.setRange(s, 1, s, 1);
                })();
            } else if (insertDom) {
                (function () {
                    var s = domUtils.createSpan();
                    s.innerHTML = CONST.FILL_CHAR;
                    splitInsert = amendUtils.splitInsertDom(endDom, endOffset, true, amendUser.getCurUser());
                    if (splitInsert.isInsert && splitInsert.split) {
                        domUtils.after(s, insertDom);
                        rangeUtils.setRange(s, 1, s, 1);
                    } else if (splitInsert.isInsert) {
                        domUtils.before(s, insertDom, endOffset > 0);
                        rangeUtils.setRange(s, 1, s, 1);
                    }
                })();
            } else if (delDom) {
                (function () {
                    var s = domUtils.createSpan();
                    s.innerHTML = CONST.FILL_CHAR;
                    splitInsert = amendUtils.splitDeletedDom(endDom, endOffset);
                    if (splitInsert) {
                        domUtils.after(s, delDom);
                        rangeUtils.setRange(s, 1, s, 1);
                    } else {
                        domUtils.before(s, delDom, endOffset > 0);
                        rangeUtils.setRange(s, 1, s, 1);
                    }
                })();
            } else if (h6Patch()) {
                utils.stopEvent(e);
                return;
            }

            sel.collapseToEnd();
            return;
        }

        splitInsert = amendUtils.splitInsertDom(endDom, endOffset, false, amendUser.getCurUser());
        amendImg = amendUtils.getWizAmendImgParent(endDom);
        if (splitInsert.isInsert && !splitInsert.split && !amendImg) {
            if (endOffset === 0 && splitInsert.insertDom.nodeType === 1) {
                //添加空字符，避免录入的字符被放到 已删除的后面
                domUtils.before(ENV.doc.createTextNode(CONST.FILL_CHAR), splitInsert.insertDom.childNodes[0]);
                rangeUtils.setRange(splitInsert.insertDom, 1, null, null);
            } else {
                rangeUtils.setRange(endDom, endOffset, null, null);
            }

            range = sel.getRangeAt(0);
            /**
             * Tab == 4 * ' '
             */
            if (keyCode === 9) {
                range.insertNode(domUtils.getTab());
                sel.modify('move', 'forward', 'character');
                sel.modify('move', 'forward', 'character');
                sel.modify('move', 'forward', 'character');
                sel.modify('move', 'forward', 'character');
                utils.stopEvent(e);
            }
            return;
        }

        nSpan = amendUtils.createDomForInsert(amendUser.getCurUser());
        if (splitInsert.split) {
            //如果在 用户新增的 span 内操作， 则在span 拆分后，添加到 两个 span 之间
            if (endDom.nodeType == 3) {
                endDom = endDom.parentNode;
            }
            domUtils.before(nSpan, endDom, endOffset > 0);
        } else if (amendImg) {
            //如果 光标处于 已修订的图片内， 则添加在 图片 容器 后面
            domUtils.after(nSpan, amendImg);
        } else if (endDom.nodeType == 1) {
            // endDom nodeType == 1 时， 光标应该是在 childNodes[endOffset] 元素的前面
            if (endOffset < endDom.childNodes.length) {
                //避免嵌套 span ，如果 endDom 为 wizSpan 并且 内容为空或 br 时，直接删除该span
                if (endDom.getAttribute(CONST.ATTR.SPAN) &&
                    (endDom.childNodes.length === 0 ||
                        (endDom.childNodes.length === 1 && domUtils.isTag(endDom.childNodes[0], 'br')))) {
                    domUtils.before(nSpan, endDom);
                    domUtils.remove(endDom);
                } else {
                    domUtils.before(nSpan, endDom.childNodes[endOffset]);
                }
            } else if (domUtils.isTag(endDom, ['td', 'th']) ||
                domUtils.hasClass(endDom, CONST.CLASS.TODO_MAIN)) {
                //如果光标处于 表格内部，不能直接把 nSpan 放到 td 的 后面
                //也不能把 nSpan 放到 todoList Main 的后面
                if (domUtils.isEmptyDom(endDom)) {
                    endDom.innerHTML = '';
                }
                endDom.appendChild(nSpan);
            } else {
                domUtils.after(nSpan, endDom);
            }

        } else if (endDom.nodeType == 3) {
            if (amendUtils.splitDeletedDom(endDom, endOffset)) {
                domUtils.after(nSpan, endDom.parentNode);

            } else if (endOffset < endDom.nodeValue.length) {
                tmpSplitStr = endDom.nodeValue.substr(endOffset);
                tmpSplit = ENV.doc.createTextNode(tmpSplitStr);
                endDom.nodeValue = endDom.nodeValue.substr(0, endOffset);
                domUtils.after([nSpan, tmpSplit], endDom);

            } else {
                nA = domUtils.getParentByTagName(endDom, 'a', true, null);
                tmpNext = nA ? domUtils.getNextNode(endDom) : null;
                tmpA = tmpNext ? domUtils.getParentByTagName(tmpNext, 'a', true, null) : null;
                nSpanNext = endDom.nextSibling;
                if (nA && nA !== tmpA) {
                    //光标在 <A> 标签结尾的时候，一定要让光标进入 <A> 下一个Dom
                    domUtils.after(nSpan, nA);
                } else if (nSpanNext) {
                    domUtils.before(nSpan, nSpanNext);
                } else {
                    endDom.parentNode.insertBefore(nSpan, null);
                }

            }
        }

        /**
         * Tab == 4 * ' '
         */
        if (keyCode === 9) {
            nSpan.appendChild(domUtils.getTab());
            rangeUtils.setRange(nSpan, 2, null, null);
            utils.stopEvent(e);
        } else {
            rangeUtils.setRange(nSpan.childNodes[0], 1, null, null);
        }

        //不能使用 selectAllChildren ，否则 输入 空格时 浏览器会自动复制前一个 span 的所有样式
        // sel.selectAllChildren(nSpan);
        //此方法会导致 Mac 的搜狗输入法 第一个字母被吃掉
        // rangeUtils.setRange(nSpan.childNodes[0], 0, nSpan.childNodes[0], nSpan.childNodes[0].nodeValue.length);

    },
    /**
     * 按下键盘 for 逆修订
     * @param e
     */
    onKeyDownReverse: function (e) {
        var keyCode = e.keyCode || e.which;
        // console.info(e);
        var sel = ENV.doc.getSelection();
        var fixed = amendUtils.fixedAmendRange();

        /**
         * Backspace
         */
        if (keyCode === 8) {
            historyUtils.saveSnap(false);

            if (sel.isCollapsed && fixed.leftDom) {
                // // 如果前一个是 table，则 delete 键直接移动光标
                // cell = domUtils.getParentByTagName(fixed.leftDom, ['td', 'th'], true, null);
                // console.log(sel.getRangeAt(0).endContainer.outerHTML);
                // console.log(sel.getRangeAt(0).endOffset);
                // console.log(fixed.leftDom);
                // if (!curCell && cell) {
                //     console.log('prev is table, stop event....')
                //     rangeUtils.setRange(cell, domUtils.getEndOffset(cell));
                //     utils.stopEvent(e);
                //     return;
                // }
                fixed.startImg = amendUtils.getWizAmendImgParent(fixed.leftDom);
                if (fixed.startImg) {
                    fixed.startDom = fixed.startImg;
                    fixed.startOffset = 0;
                    rangeUtils.setRange(fixed.startDom, fixed.startOffset, fixed.endDom, fixed.endOffset);
                } else if (fixed.leftDom.nodeType === 3 &&
                    fixed.leftDom.nodeValue.length == 1) {
                    fixClearLine(fixed.leftDom, -1);
                }
            }
            return;
        }
        /**
         * Delete
         */
        if (keyCode === 46) {
            historyUtils.saveSnap(false);
            if (sel.isCollapsed && fixed.rightDom) {
                // // 如果下一个是 table，则 delete 键直接移动光标
                // var cell = domUtils.getParentByTagName(fixed.rightDom, ['td', 'th'], true, null);
                // if (!curCell && cell) {
                //     console.log('next is table, stop event....')
                //     rangeUtils.setRange(cell, 0);
                //     utils.stopEvent(e);
                //     return;
                // }
                fixed.endImg = amendUtils.getWizAmendImgParent(fixed.rightDom);
                if (fixed.endImg) {
                    fixed.endDom = fixed.endImg;
                    fixed.endOffset = fixed.endImg.childNodes.length;
                    rangeUtils.setRange(fixed.startDom, fixed.startOffset, fixed.endDom, fixed.endOffset);
                } else if (fixed.rightDom.nodeType === 3 &&
                    fixed.rightDom.nodeValue.length == 1) {
                    fixClearLine(fixed.rightDom, 1);
                }
            }
            return;
        }

        /**
         * 其他功能键一概忽略
         */
        if (utils.checkNonTxtKey(e)) {
            return;
        }

        /**
         * 普通字符
         */
        historyUtils.saveSnap(false);
        amend.splitAmendDomByRange(fixed);
        //删除 range 选中区域后，必须要避免光标进入 table 容器内
        var check = tableUtils.checkCaretInTableContainer();
        blockUtils.insertEmptyLine(check.tableContainer, check.after);

        if (keyCode === 13 && h6Patch()) {
            utils.stopEvent(e);
            return;
        }

        function fixClearLine (dom, direct) {
            //从右往左 Backspace  direct = -1
            //从左往右 Delete  direct = 1
            if (!dom) {
                return;
            }
            var tmpDom, wizDom;
            //专门处理 删除一行文字后， 浏览器默认记住最后删除文字样式的特性
            //此特性导致删除修订内容后， 重新输入的文字会带有修订的样式
            wizDom = amendUtils.getWizAmendParent(dom);
            if (wizDom && wizDom.childNodes.length === 1) {
                tmpDom = domUtils.createSpan();
                tmpDom.innerHTML = CONST.FILL_CHAR + CONST.FILL_CHAR;
                domUtils.before(tmpDom, wizDom, direct > 0);
                domUtils.remove(wizDom);
                rangeUtils.setRange(tmpDom, direct > 0 ? 0 : 2, tmpDom, 1);
            }
        }
    },
    /**
     * 避免 修订信息图层被编辑  & 鼠标按下后 暂停 amendInfo 显示
     * @param e
     */
    onMouseDown: function (e) {
        var isInfo = amendInfo.isInfo(e.target);
        if (isInfo) {
            utils.stopEvent(e);
        }
        amendInfo.stop();
    },
    /**
     *  鼠标按下后 恢复 amendInfo 显示
     * @param e
     */
    onMouseUp: function (e) {
        amendInfo.start();
        //var amendDoms = amendUtils.getSelectedAmendDoms();
        //
        //if (amendDoms) {
        //    console.log(amendDoms)
        //
        //    //amendInfo.showAmendsInfo(amendDoms);
        //}
    },
    onTouchStart: function (e) {
    }
};

module.exports = amend;

function h6Patch () {
    var range, block, hObj, newLine, isLast = false;
    // 对于 h6 在行尾 换行会导致下一行还是 h6
    range = rangeUtils.getRange();
    block = ENV.doc.queryCommandValue("formatBlock");
    if (/^h[1-6]+$/i.test(block) &&
        range && range.startOffset === domUtils.getEndOffset(range.startContainer)) {
        hObj = domUtils.getParentByTagName(range.startContainer, block, true);
        isLast = isLastDom(hObj, range.startContainer);
    }
    if (isLast && hObj) {
        newLine = ENV.doc.createElement('div');
        newLine.appendChild(ENV.doc.createElement('br'));
        domUtils.after(newLine, hObj);
        rangeUtils.setRange(newLine, 0);
        return true;
    }
    return false;

    function isLastDom (parent, dom) {
        if (!parent) {
            return false;
        }
        var lastDom = domUtils.getLastDeepChild(parent);
        var p = domUtils.getParentByFilter(lastDom, function (obj) {
            return obj === dom;
        }, true);

        return !!p;
    }
}
},{"../blockUtils/blockUtils":13,"../codeUtils/codeCore":14,"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/lang":22,"../common/utils":24,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"../tableUtils/tableCore":52,"../tableUtils/tableUtils":54,"../todoUtils/todoCore":56,"./amendInfo":8,"./amendUser":9,"./amendUtils/amendExtend":11}],8:[function(require,module,exports){
/**
 * 修订信息显示图层 相关对象
 */

var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    Lang = require('../common/lang'),
    LANG = Lang.getLang(),
    domUtils = require('../domUtils/domBase'),
    amendUtils = require('./amendUtils/amendBase'),
    amendUser = require('./amendUser'),
    userAction = require('../common/wizUserAction');

var callback = {
    onAccept: null,
    onRefuse: null
};
//暂停显示的标志
var pause = false,
//记录最后一次鼠标移动的位置
    lastMousePos = {x: null, y: null};


var amendInfo = {
    cur: null,
    curPos: null,
    isMulti: false,
    isSelection: false,
    template: null,
    main: null,
    img: null,
    name: null,
    content: null,
    time: null,
    btnAccept: null,
    btnRefuse: null,

    /**
     * 修订信息 显示图层 初始化
     * @param options  {readonly: boolean,  cb: {onAccept: function, onRefuse: function}}
     * @param cb
     */
    init: function (options, cb) {
        amendInfo.template = ENV.doc.createElement('div');
        amendInfo.main = createAmendInfo();
        amendInfo.readonly = !!(options && options.readonly);

        domUtils.setContenteditable(amendInfo.main, false);

        if (cb && cb.onAccept) {
            callback.onAccept = cb.onAccept;
        }
        if (cb && cb.onRefuse) {
            callback.onRefuse = cb.onRefuse;
        }
        _event.unbind();
        _event.bind();
    },
    /**
     * 删除 修订信息 图层
     */
    remove: function () {
        _event.unbind();
        removeAmendInfo();
        amendInfo.main = null;
        amendInfo.img = null;
        amendInfo.name = null;
        amendInfo.content = null;
        amendInfo.time = null;
        amendInfo.btnAccept = null;
        amendInfo.btnRefuse = null;
    },
    /**
     * 显示 修订信息
     * @param dom
     * @param pos
     */
    show: function (dom, pos) {
        clearTimeout(amendInfo.showTimer);
        clearTimeout(amendInfo.hideTimer);

        var isSelection = utils.isArray(dom),
            isMulti = isSelection && dom.length > 1,
            cur = !isSelection ? dom : (isMulti ? null : dom[0]),
            showFlag = false;

        amendInfo.isSelection = isSelection;
        if ((amendInfo.isMulti !== isMulti) ||
            (cur !== amendInfo.cur)) {

            //移动到不同的 dom 时，立刻隐藏当前标签， 等待固定时间后再显示信息
            amendInfo.hide(true);

            showFlag = true;
        } else if (!amendInfo.curPos || Math.abs(amendInfo.curPos.left - pos.left) > 75 ||
            Math.abs(amendInfo.curPos.top - pos.top) > 24) {
            //在同一个 dom 内移动距离较远后， 更换信息图层位置
            showFlag = true;
        }

        if (showFlag) {
            amendInfo.showTimer = setTimeout(function () {
                amendInfo.isMulti = isMulti;
                amendInfo.cur = cur;
                showInfo(pos);
            }, CONST.AMEND.INFO_TIMER * 2);
        }
    },
    /**
     * 隐藏 修订信息
     * @param quick
     */
    hide: function (quick) {
        clearTimeout(amendInfo.showTimer);
        clearTimeout(amendInfo.hideTimer);
        if (!amendInfo.cur && !amendInfo.isMulti) {
            return;
        }

        if (quick) {
            hideInfo();
        } else {
            amendInfo.hideTimer = setTimeout(hideInfo, CONST.AMEND.INFO_TIMER);
        }
    },
    /**
     * 判断 dom 是否 amendInfo layer 内的元素（包括layer）
     * @param dom
     */
    isInfo: function (dom) {
        var amendInfoMain = domUtils.getParentByFilter(dom, function (node) {
            return node == amendInfo.main;
        }, true);
        return !!amendInfoMain;
    },
    /**
     * 恢复 info 的显示
     */
    start: function () {
        pause = false;
    },
    /**
     * 暂停 info 的显示
     */
    stop: function () {
        amendInfo.hide(true);
        pause = true;
    }
};

var _event = {
    bind: function () {
        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            ENV.event.add(CONST.EVENT.ON_TOUCH_START, _event.handler.onTouchstart);
            ENV.event.add(CONST.EVENT.ON_TOUCH_END, _event.handler.onMouseMove);
        } else {
            ENV.event.add(CONST.EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
        }
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_TOUCH_START, _event.handler.onTouchstart);
        ENV.event.remove(CONST.EVENT.ON_TOUCH_END, _event.handler.onMouseMove);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
    },
    bindInfoBtn: function () {
        _event.unbindInfoBtn();
        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            amendInfo.main.addEventListener('touchend', _event.handler.onClick);
        } else {
            amendInfo.main.addEventListener('click', _event.handler.onClick);
        }

    },
    unbindInfoBtn: function () {
        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            amendInfo.main.removeEventListener('touchend', _event.handler.onClick);
        } else {
            amendInfo.main.removeEventListener('click', _event.handler.onClick);
        }
    },

    handler: {
        /**
         * 检测 鼠标移动到的 dom 对象，是否需要显示 或 隐藏 amendInfo
         * @param e
         */
        onMouseMove: function (e) {
            //console.log('onMouseMove....')
            var eventClient = utils.getEventClientPos(e);
            //如果鼠标没有移动， 仅仅输入文字导致触发mousemove事件时，不弹出信息框
            if (lastMousePos.x === eventClient.x && lastMousePos.y === eventClient.y) {
                return;
            }
            lastMousePos = eventClient;
            if (pause) {
                return;
            }
            var target = e.target,
                isInfo = amendInfo.isInfo(target),
                scroll,
                pos = {
                    width: 20,
                    height: 20
                };

            //在 修订信息图层内移动， 不进行任何操作
            if (isInfo) {
                clearTimeout(amendInfo.showTimer);
                clearTimeout(amendInfo.hideTimer);
                return;
            }

            var sel = ENV.doc.getSelection(),
                selectedDoms,
                targetDom = amendUtils.getWizDeleteParent(target) || amendUtils.getWizInsertParent(target);

            if (!sel.isCollapsed && targetDom && sel.containsNode(targetDom, true)) {
                //有选择区域， 且 target 在选择区域内
                selectedDoms = sel.isCollapsed ? null : amendUtils.getAmendDoms({
                    selection: true,
                    selectAll: false
                });
            }

            //校验选择区域内是否有多个dom
            if (selectedDoms) {
                selectedDoms = selectedDoms.deletedInsertList.concat(selectedDoms.insertList, selectedDoms.deleteList);
                //选择多个修订内容时，不显示详细信息
                if (selectedDoms.length === 0) {
                    selectedDoms = null;
                    //} else if (selectedDoms.length == 1) {
                    //    targetDom = selectedDoms[0];
                    //    selectedDoms = null;
                }
            }
            var fontSize;
            if (selectedDoms || targetDom) {
                fontSize = parseInt(ENV.win.getComputedStyle(targetDom)['font-size']);
                if (isNaN(fontSize)) {
                    fontSize = 14;
                }
                scroll = domUtils.getPageScroll();
                pos.left = eventClient.x + scroll.left;
                pos.top = eventClient.y + scroll.top - fontSize;
                if (pos.top < targetDom.offsetTop) {
                    pos.top = targetDom.offsetTop;
                }
                amendInfo.show(selectedDoms || targetDom, pos);
            } else {
                amendInfo.hide(false);
            }
        },
        onTouchstart: function (e) {
            //console.log('onTouchstart....')
            var target = e.target,
                isInfo = amendInfo.isInfo(target);
            if (isInfo) {
                return;
            }
            amendInfo.hide(false);
        },

        onClick: function (e) {
            var target;
            if (e.changedTouches) {
                target = e.changedTouches[0].target;
            } else {
                target = e.target;
            }
            if (target.id == CONST.ID.AMEND_INFO_ACCEPT) {
                _event.handler.onAccept(e);
            } else if (target.id == CONST.ID.AMEND_INFO_REFUSE) {
                _event.handler.onRefuse(e);
            }
            utils.stopEvent(e);
        },
        onAccept: function (e) {
            if (callback.onAccept) {
                callback.onAccept(getCallbackParams());
            }
            amendInfo.hide(true);
            userAction.save(userAction.ActionId.ClickAcceptFromAmendInfo);
        },
        onRefuse: function (e) {
            if (callback.onRefuse) {
                callback.onRefuse(getCallbackParams());
            }
            amendInfo.hide(true);
            userAction.save(userAction.ActionId.ClickRefuseFromAmendInfo);
        }
    }
};

/**
 * 创建 修订信息 图层
 */
function createAmendInfo() {
    var mask = ENV.doc.getElementById(CONST.ID.AMEND_INFO), container;
    domUtils.remove(mask);

    mask = ENV.doc.createElement('div');
    container = ENV.doc.createElement('div');
    domUtils.setContenteditable(container, false);
    mask.appendChild(container);
    mask.id = CONST.ID.AMEND_INFO;
    domUtils.css(mask, {
        'position': 'absolute',
        'z-index': CONST.CSS.Z_INDEX.amendInfo,
        'display': 'none',
        'padding': '6px',
        'font-family': '"Microsoft Yahei","微软雅黑",Helvetica,SimSun,SimHei'
    });
    container.innerHTML = getInfoTemplate();

    domUtils.css(container, {
        'background-color': 'white',
        'padding': '0px',
        'font-size': '12px',
        'border': '1px solid #D8D8D8',
        '-webkit-border-radius': '4px',
        '-moz-border-radius': '4px',
        '-border-radius': '4px',
        '-webkit-box-shadow': 'rgba(0, 0, 0, 0.24) 0px 3px 3px',
        '-moz-box-shadow': 'rgba(0, 0, 0, 0.24) 0px 3px 3px',
        'box-shadow': 'rgba(0, 0, 0, 0.24) 0px 3px 3px',
        'min-width': '160px',
        'max-width': '280px',
        'min-height': '50px'
    });

    amendInfo.template.appendChild(mask);

    return mask;
}

function getInfoTemplate() {
    if (ENV.client.type.isIOS || ENV.client.type.isMac || ENV.client.type.isAndroid) {
        return '<div id="' + CONST.ID.AMEND_INFO_SINGLE + '" style="display:none; padding: 8px 16px;">' +
            '<img id="' + CONST.ID.AMEND_INFO_IMG + '" class="' + CONST.CLASS.IMG_NOT_DRAG + '" style="width: 40px; height: 40px !important; position: absolute; -webkit-border-radius: 40px;-moz-border-radius:40px;border-radius:40px;">' +
            '<ul style="list-style-type: none;margin: 4px 0 0 50px;padding-left: 0;"><li style="line-height: 18px;white-space: nowrap;padding: 2px 0;">' +
            '<span id="' + CONST.ID.AMEND_INFO_NAME + '" style="color:#000;font-size:12px;font-weight:bold;max-width:90px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:inline-block;"></span>' +
            '<span id="' + CONST.ID.AMEND_INFO_CONTENT + '" style="color:#000;font-size:12px;margin-left:.5em;display:inline-block;overflow:hidden;float:right"></span>' +
            '</li><li style="line-height: 18px;text-align: right;">' +
            '<span id="' + CONST.ID.AMEND_INFO_TIME + '" style="color:#A3A3A3;font-size:12px;"></span></li></ul>' +
            '</div>' +
            '<div id="' + CONST.ID.AMEND_INFO_MULTI + '" style="display:none; padding: 8px 16px;">' +
            '<p style="margin: 4px 16px;">' + LANG.Amend.MultiInfo + '</p>' +
            '</div>' +
            '<div id="' + CONST.ID.AMEND_INFO_TOOLS + '" style="padding:0;margin:0;box-sizing: border-box;">' +
            '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;">' +
            '<a id="' + CONST.ID.AMEND_INFO_REFUSE + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + LANG.Amend.BtnRefuse + '</a></div>' +
            '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;">' +
            '<a id="' + CONST.ID.AMEND_INFO_ACCEPT + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + LANG.Amend.BtnAccept + '</a></div>' +
            '</div>';
    }

    //if (ENV.client.type.isWeb || ENV.client.type.isWin) {
    return '<div id="' + CONST.ID.AMEND_INFO_SINGLE + '" style="display:none; padding: 8px 16px;">' +
        '<img id="' + CONST.ID.AMEND_INFO_IMG + '" class="' + CONST.CLASS.IMG_NOT_DRAG + '" style="width: 40px; height: 40px !important; position: absolute;">' +
        '<ul style="list-style-type: none;margin: 4px 0 0 50px;padding-left: 0;"><li style="line-height: 18px;white-space: nowrap;padding: 2px 0;">' +
        '<span id="' + CONST.ID.AMEND_INFO_NAME + '" style="color:#000;font-size:12px;font-weight:bold;max-width:90px;overflow:hidden;text-overflow:ellipsis;white-space:nowrap;display:inline-block;"></span>' +
        '<span id="' + CONST.ID.AMEND_INFO_CONTENT + '" style="color:#000;font-size:12px;margin-left:.5em;display:inline-block;overflow:hidden;float:right"></span>' +
        '</li><li style="line-height: 18px;text-align: right;">' +
        '<span id="' + CONST.ID.AMEND_INFO_TIME + '" style="color:#A3A3A3;font-size:12px;"></span></li></ul>' +
        '</div>' +
        '<div id="' + CONST.ID.AMEND_INFO_MULTI + '" style="display:none; padding: 8px 16px;">' +
        '<p style="margin: 4px 16px;">' + LANG.Amend.MultiInfo + '</p>' +
        '</div>' +
        '<div id="' + CONST.ID.AMEND_INFO_TOOLS + '" style="padding:0;margin:0;box-sizing:border-box;border-top:1px solid #D8D8D8">' +
        '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;border-right: 1px solid #D8D8D8">' +
        '<a id="' + CONST.ID.AMEND_INFO_ACCEPT + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + LANG.Amend.BtnAccept + '</a></div>' +
        '<div style="line-height: 26px;width: 50%;display:inline-block;text-align: center;padding:0 8px;margin:0;box-sizing: border-box;">' +
        '<a id="' + CONST.ID.AMEND_INFO_REFUSE + '" href="javascript:void(0);" style="font-size:12px;display:block;cursor:pointer;color:#447BD8;text-decoration: blink;">' + LANG.Amend.BtnRefuse + '</a></div>' +
        '</div>';
    //}


}

function getCallbackParams() {
    return {
        dom: amendInfo.cur,
        isSelection: !!amendInfo.isSelection
    };
}

function initUserInfo(pos) {
    var dom = amendInfo.cur,
        guid = dom.getAttribute(CONST.ATTR.SPAN_USERID),
        user = amendUser.getUserByGuid(guid),
        name = user ? user.name : LANG.Amend.UserNameDefault,
        time = dom.getAttribute(CONST.ATTR.SPAN_TIMESTAMP),
        isDelete = !!dom.getAttribute(CONST.ATTR.SPAN_DELETE),
        user = amendUser.getUserByGuid(guid);
    time = time.substring(0, time.length - 3);

    amendInfo.curPos = pos;
    amendInfo.img.src = user ? user.imgUrl : '';
    amendInfo.name.innerText = name;
    amendInfo.name.setAttribute('title', name);
    amendInfo.content.innerText = isDelete ? LANG.Amend.Delete : LANG.Amend.Edit;
    amendInfo.time.innerText = time;

    amendInfo.multiUser.style.display = 'none';
    amendInfo.singleUser.style.display = 'block';
}

function initMultiInfo(pos) {
    amendInfo.curPos = pos;
    amendInfo.singleUser.style.display = 'none';
    amendInfo.multiUser.style.display = 'block';
}

function showInfo(pos) {

    if (amendInfo.main.parentNode == amendInfo.template) {
        ENV.doc.body.appendChild(amendInfo.main);
        amendInfo.singleUser = ENV.doc.getElementById(CONST.ID.AMEND_INFO_SINGLE);
        amendInfo.multiUser = ENV.doc.getElementById(CONST.ID.AMEND_INFO_MULTI);
        amendInfo.img = ENV.doc.getElementById(CONST.ID.AMEND_INFO_IMG);
        amendInfo.name = ENV.doc.getElementById(CONST.ID.AMEND_INFO_NAME);
        amendInfo.content = ENV.doc.getElementById(CONST.ID.AMEND_INFO_CONTENT);
        amendInfo.time = ENV.doc.getElementById(CONST.ID.AMEND_INFO_TIME);
        amendInfo.tools = ENV.doc.getElementById(CONST.ID.AMEND_INFO_TOOLS);
        amendInfo.btnAccept = ENV.doc.getElementById(CONST.ID.AMEND_INFO_ACCEPT);
        amendInfo.btnRefuse = ENV.doc.getElementById(CONST.ID.AMEND_INFO_REFUSE);
    }

    if (amendInfo.cur) {
        initUserInfo(pos);
    } else {
        initMultiInfo(pos);
    }

    _event.bindInfoBtn();

    if (amendInfo.readonly) {
        amendInfo.tools.style.display = 'none';
    } else {
        amendInfo.tools.style.display = 'block';
    }

    domUtils.css(amendInfo.main, {
        'top': '0px',
        'left': '0px',
        'display': 'block',
        'visibility': 'hidden'
    });
    domUtils.setLayout({
        layerObj: amendInfo.main,
        target: pos,
        layout: CONST.TYPE.POS.upLeft,
        fixed: false,
        noSpace: false,
        reverse: true
        //reverse: !ENV.client.type.isPhone
    });
    domUtils.css(amendInfo.main, {
        'display': 'block',
        'visibility': 'visible'
    });
}

function hideInfo() {
    if (amendInfo.main) {
        _event.unbindInfoBtn();
        amendInfo.cur = null;
        amendInfo.curPos = null;
        amendInfo.isMulti = false;
        amendInfo.isSelection = false;
        amendInfo.img.src = '';
        amendInfo.name.innerText = '';
        amendInfo.name.setAttribute('title', '');
        amendInfo.content.innerText = '';
        domUtils.css(amendInfo.main, {
            'display': 'none'
        });
        amendInfo.template.appendChild(amendInfo.main);
    }
}

/**
 * 删除 修订信息 图层
 */
function removeAmendInfo() {
    domUtils.remove(ENV.doc.getElementById(CONST.ID.AMEND_INFO));
}

module.exports = amendInfo;
},{"../common/const":18,"../common/env":20,"../common/lang":22,"../common/utils":24,"../common/wizUserAction":26,"../domUtils/domBase":28,"./amendUser":9,"./amendUtils/amendBase":10}],9:[function(require,module,exports){
/**
 * 用于记录 当前操作者的信息
 * @type {{guid: string, hash: string, name: string, color: string, init: Function}}
 */

var ENV = require('../common/env'),
    CONST = require('../common/const'),
    domUtils = require('../domUtils/domBase');

var DefaultImg = 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAAC0AAAAtCAYAAAA6GuKaAAAExUlEQVRYR9WZz08bRxTHvya1CHbUSHXCiZzCIaaBBOUUEveQSORWYwwRxRCpqtIqjZL8XQkEUppAbQMX6hgh5UetRiFV4MABQXoK2AgUx1vNzs7u7Hpmf3m3Vc3Blm2Gz/vOd95784goiqLgf/aIhAVdrVaxu7sLKomC7u7uwKQJBPrFi5eoVvfx6VMdZONOnIhjf78KBQoINQEnrwcuD+DkyS9bhvcNfXR0hFKppEKqWqp8BE0l1N7XgPXvKOjq6kJfX29L4L6hCfDh4aEnYBbQjRuD+PhxD4nEV77gfUEvLy/rXm1S2GIJQ3VtJ9TdoFtDfuKxOK5fv+YJ3jM0BfZmCd4y5tcUPp3+Njzo1dVV1Go1X5Yw+Z1oTOLWVI91HMfg4KBrcE9KLy0tq+mrVUvwwGSxzs5ODAxcDgt6yXWWcLIEVd5QezgzFA704iKBdk5rQmCLJdRzwYEPD2fCgS4WF408LMkSooOmhsl5mBYcTmlFQTY7HA50oVi0LRxeLcHDj4QGXShKK50jsGpfsyWY2uR5dCQbjtL5QsHUSzgVDidL8AcxNOiVlRXUagdccTEfSqGfLVmCt4RRpBTcHB0JR+lS6Tn29va04iIBdsgSvCVoxlMQ/SKKTCYdPPT29jb+qFSEB5HvJbxYgnqctq2958+jpyfpCtx1RVxY+M1eYUsKYyrqAWmf85ZgwAx+bOxm8ND0D4rLOK+wXZbQg+F6bALfQAOj2Syi0agjuGul5+cXTA1+EJbg7UFeZ4bS6OjoCA762fy87udWsoTMHkT43PiYIzD5gmul19bW8OHD36Yy7tUSVg/T8q6VeCjIjX8XLDRZ/OkzojZNdX6zhNUSzOPEGrFYLFhotlpxcQkHBwdaZTS3l7LCYWeJeDyGoTBvLgy8XC5jl1jFppeQZQm/luC3wLWn+V9qNBqYm/u1qb3kewmZDXjVe5JJ9PdfdGWJlqHJAk9+mQOBl1nC6dCRTZrIuTt41qh8Kc0WmZmZ1dXmFbTzMCtOly71I3nunGeVPaU80eo7Ozv4vfRcvzY1WULrK8wBqW7HRG7cF3DL0GSBqenHRk9iKc3WQ8eGka0ABwZtpzADNZ4VTE7kfKscCPSjqWm9vZRnDGoJVv1uTf7H0A8fTen24DOGSGH23q3JiX9f6Xq9jnfv/sKfb954BmbTqYsX+nDmTBcSiYTnAFynvK2tLZDhef3zZ60S0huHLNXxHrZTnXwWiUTQffYsUqkrrgKQQpMLbD6fF+ZhN4WD9zB/cTC/ZsN3YwysN1CZNE6fOiUMwgT99u061tfXjRmb4IrkpnCIgOVBNAObdg/A1z1JfJO6qgegQ5fLqyDFwnqnY/M2L4WD75GZh5nCIqWlFqOyq7sdaWvDnZ9uq+A69Ozsk1AUtt4rhd2fNkpg4wWzULT9JZ/d/uF7tLe3U+iNjU1tPGAMBu1uzbJKZ1W4FUuIhpQX+nqRSl2l0O/fb6BSqZhbTX32xmUJm14iVGCN5VjbMfx850cK/erVa2xuburb4LaBl6U1mSVE6zpZwphhE4s0cP/eXQqdzxewX61Kcy6bS9ComPLm0ixKa34PXXMyoMDkoUNPP56Rt5fcITGUEgMH6mGLPdl/fh/cv4t/ANultPKz243RAAAAAElFTkSuQmCC';

var AmendUser = function (userInfo) {
    if (!userInfo) {
        userInfo = {};
    }
    this.guid = userInfo.user_guid || '';
    this.hash = getHash(this.guid);
    this.name = userInfo.user_name || '';
    this.imgUrl = userInfo.img_url ? userInfo.img_url : getImgUrl(this.guid);
    this.color = userInfo.color || createUserColor(this);
};
var curAmendUser = null;
var userDom = null;
var users = null,       //用户 AmendUser 对象集合，用于 修订功能
    usersForSave = null;//用户 保存数据，用于保存到 meta 内

var amendUserUtils = {
    initUser: function (userInfo) {
        //初始化用户信息， 保证第一个修订用户的信息能被正常保存
        loadUsers();

        if (!userInfo) {
            return null;
        }

        curAmendUser = new AmendUser(userInfo);
        addUser(curAmendUser);
    },
    getCurUser: function () {
        saveUser();
        return curAmendUser;
    },
    getUserByGuid: function (guid) {
        if (curAmendUser && guid === curAmendUser.guid) {
            return curAmendUser;
        }
        if (users && users[guid]) {
            return users[guid];
        }
        loadUsers();
        return users[guid];
    },
    // /**
    //  * 删除 修订颜色数据（用于确认修订）
    //  */
    // removeAllUserInfo: function () {
    //     var d = ENV.doc.getElementById(CONST.ID.AMEND_USER_INFO);
    //     domUtils.remove(d);
    //
    //     userDom = null;
    //     users = null;
    //     usersForSave = null;
    // },
    setUsersData: function (_usersData) {
        var i, j, u, u1, u2;
        if (!_usersData) {
            return;
        }
        for (i = 0, j = _usersData.length; i < j; i++) {
            u = _usersData[i];
            u1 = users[u.user_guid];
            u2 = usersForSave[u.user_guid];
            if (u1 && u.user_name) {
                u1.name = u.user_name;
            }
            if (u1 && u.img_url) {
                u1.imgUrl = u.img_url;
            }
            if (u2 && u.user_name) {
                u2.name = u.user_name;
            }
        }
    }
};

function getHash(guid) {
    //hash = util.getHash(userInfo.user_guid);
    //hash = '_w' + hash;
    return guid;
}

function getImgUrl(guid) {
    if (ENV.client.type.isWeb) {
        return '/wizas/a/users/avatar/' +
            guid +
            '?default=true&_' + new Date().valueOf();
    } else if (ENV.client.type.isWin) {
        try {
            var avatarFileName = external.GetAvatarByUserGUID(guid);
            return avatarFileName ? avatarFileName : DefaultImg;
        } catch (e) {
            console.error(e);
        }
    } else if (ENV.client.type.isMac) {

    } else if (ENV.client.type.isIOS) {

    } else if (ENV.client.type.isAndroid) {

    }

    return DefaultImg;
}
/**
 * 从客户端根据 guid 获取最新的用户昵称， 保证显示最新的用户昵称
 * @param guid
 * @returns {*}
 */
function getUserNameFromClient(guid) {
    if (ENV.client.type.isWeb) {

    } else if (ENV.client.type.isWin) {
        try {
            return external.GetAliasByUserGUID(guid);
        } catch (e) {
            console.error(e);
        }
    } else if (ENV.client.type.isMac) {

    } else if (ENV.client.type.isIOS) {

    } else if (ENV.client.type.isAndroid) {

    }

    return null;
}

function getUserDom() {
    if (userDom) {
        return userDom;
    }
    userDom = ENV.doc.getElementById(CONST.ID.AMEND_USER_INFO);
    return userDom;
}
function createUserDom() {
    userDom = ENV.doc.createElement('meta');
    userDom.id = CONST.ID.AMEND_USER_INFO;
    userDom.name = CONST.ID.AMEND_USER_INFO;
    ENV.doc.getElementsByTagName('HEAD')[0].insertBefore(userDom, null);
}

function loadUsers() {
    if (users) {
        return;
    }
    var i, u, tmpName;

    users = {};
    usersForSave = {};

    userDom = getUserDom();
    if (!userDom) {
        return;
    }

    try {
        //根据已有数据获取曾经修订过的用户信息
        usersForSave = JSON.parse(userDom.content);

        for (i in usersForSave) {
            if (usersForSave.hasOwnProperty(i)) {
                u = usersForSave[i];
                u.user_guid = i;
                tmpName = getUserNameFromClient(i);
                if (tmpName) {
                    u.user_name = tmpName;
                } else {
                    u.user_name = u.name;
                }

                users[i] = new AmendUser(u);
            }
        }
    } catch (e) {
    }
}

/**
 * 根据 user 信息生成 修订颜色
 * @param user
 */
function createUserColor(user) {
    var userKey = user.hash,
        colorCount = CONST.COLOR.length,
        tmpColors = {}, i, c;

    loadUsers();
    //如果该用户已有修订记录，直接使用
    if (users[userKey]) {
        return users[userKey].color;
    }

    //初始化 颜色列表，确认哪些颜色已被使用
    for (i in users) {
        if (users.hasOwnProperty(i)) {
            c = users[i].color;
            tmpColors[c] = true;
        }
    }

    for (i = 0; i < colorCount; i++) {
        c = CONST.COLOR[i];
        if (!tmpColors[c]) {
            return c;
        }
    }
    //如果所有颜色都被使用， 则直接使用第一种颜色
    return CONST.COLOR[0];
}

function addUser(user) {
    //如果已经存在，则替换数据
    users[user.guid] = user;
    usersForSave[user.guid] = {
        color: user.color,
        name: user.name
    };
}
function saveUser() {
    if (!userDom) {
        createUserDom();
    }

    userDom.content = JSON.stringify(usersForSave);
}


module.exports = amendUserUtils;
},{"../common/const":18,"../common/env":20,"../domUtils/domBase":28}],10:[function(require,module,exports){
/**
 * amend 中通用的基本方法集合（基础操作，以读取为主）
 *
 */

var ENV = require('../../common/env'),
    CONST = require('../../common/const'),
    utils = require('../../common/utils'),
    domUtils = require('../../domUtils/domBase'),
    rangeUtils = require('../../rangeUtils/rangeBase');

var amendUtils = {
    /**
     * 根据条件 获取 修订的 dom 集合
     * @param options  {{[selection]: Boolean, [domList]: Array, [selectAll]: Boolean}}
     * @returns {{insertList: Array, deleteList: Array, deletedInsertList: Array}}
     */
    getAmendDoms: function (options) {
        var i, j, d,
            insertAttr = {},
            deleteAttr = {},
            result = {
                insertList: [],
                deleteList: [],
                deletedInsertList: []
            }, tmp = [];
        if (options.selection) {
            insertAttr[CONST.ATTR.SPAN_INSERT] = '';
            result.insertList = amendUtils.getWizSpanFromRange(options.selectAll, insertAttr);
            //清理出 删除&新增内容
            result.deletedInsertList = domUtils.removeListFilter(result.insertList, function (dom) {
                return (dom.getAttribute(CONST.ATTR.SPAN_DELETE));
            });
            deleteAttr[CONST.ATTR.SPAN_DELETE] = '';
            result.deleteList = amendUtils.getWizSpanFromRange(options.selectAll, deleteAttr);
            //清理出 删除&新增内容
            tmp = domUtils.removeListFilter(result.deleteList, function (dom) {
                return (dom.getAttribute(CONST.ATTR.SPAN_INSERT));
            });
            //合并从 insert & delete 集合中 清理出来的 删除&新增内容
            result.deletedInsertList = utils.removeDup(result.deletedInsertList.concat(tmp));

        } else {
            for (i = 0, j = options.domList.length; i < j; i++) {
                d = options.domList[i];
                if (d.getAttribute(CONST.ATTR.SPAN_DELETE) && d.getAttribute(CONST.ATTR.SPAN_INSERT)) {
                    result.deletedInsertList.push(d);
                } else if (d.getAttribute(CONST.ATTR.SPAN_DELETE)) {
                    result.deleteList.push(d);
                } else if (d.getAttribute(CONST.ATTR.SPAN_INSERT)) {
                    result.insertList.push(d);
                }
            }
        }
        return result;
    },
    /**
     * 获取 与目标 连续且时间戳相近的 修订 dom 集合(必须同一用户的操作)
     * @param dom
     * @returns {Array}
     */
    getSameTimeStampDom: function (dom) {
        if (!dom || dom.nodeType != 1) {
            return [];
        }
        var result = [];

        findWizSibling(dom, true, result);
        result.push(dom);
        findWizSibling(dom, false, result);
        return result;

        function findWizSibling(target, isPrev, result) {
            var wizAmend, tmp, amendTypeTmp,
                amendType = getAmendType(target),
                time = target.getAttribute(CONST.ATTR.SPAN_TIMESTAMP),
                userId = target.getAttribute(CONST.ATTR.SPAN_USERID);
            if (!time) {
                return;
            }
            var sibling = getSibling(target, isPrev);
            while (sibling) {
                wizAmend = amendUtils.getWizInsertParent(sibling) || amendUtils.getWizDeleteParent(sibling);
                sibling = wizAmend;
                //首先判断是否同一用户
                if (sibling && sibling.getAttribute(CONST.ATTR.SPAN_USERID) !== userId) {
                    sibling = null;
                } else if (sibling) {
                    tmp = sibling.getAttribute(CONST.ATTR.SPAN_TIMESTAMP);
                    amendTypeTmp = getAmendType(sibling);
                    //时间相近的算法必须要考虑 删除其他用户新增的情况， 如果目标是（delete & insert）的情况，则相邻的也必须满足
                    if (amendType === amendTypeTmp && utils.isSameAmendTime(sibling.getAttribute(CONST.ATTR.SPAN_TIMESTAMP), time)) {
                        if (isPrev) {
                            result.splice(0, 0, sibling);
                        } else {
                            result.push(sibling);
                        }
                        sibling = getSibling(sibling, isPrev);
                    } else {
                        sibling = null;
                    }
                }
            }
        }

        function getAmendType(obj) {
            if (obj.getAttribute(CONST.ATTR.SPAN_DELETE) && obj.getAttribute(CONST.ATTR.SPAN_INSERT)) {
                return 1;
            } else if (obj.getAttribute(CONST.ATTR.SPAN_INSERT)) {
                return 2;
            } else if (obj.getAttribute(CONST.ATTR.SPAN_DELETE)) {
                return 3;
            }
            return 0;
        }

        function getSibling(target, isPrev) {
            return isPrev ? domUtils.getPreviousNode(target, false, null) : domUtils.getNextNode(target, false, null);
        }

    },
    /**
     * 获取选择范围内 修订 dom 集合
     * @returns {*}
     */
    getSelectedAmendDoms: function () {
        var sel = ENV.doc.getSelection(),
            range = sel.getRangeAt(0),
            startDom, endDom, startOffset, endOffset;

        var amends = amendUtils.getAmendDoms({
            selection: true,
            selectAll: false
        });
        if (amends.insertList.length === 0 && amends.deleteList.length === 0
            && amends.deletedInsertList.length === 0) {
            return null;
        }

        if (sel.isCollapsed) {
            //光标折叠状态时，不需要对 span 进行拆分
            return amends;
        }

        startDom = range.startContainer;
        startOffset = range.startOffset;
        endDom = range.endContainer;
        endOffset = range.endOffset;

        var start = checkStart(amends.deleteList, startDom, startOffset);
        if (!start) {
            start = checkStart(amends.insertList, startDom, startOffset);
            if (!start) {
                start = checkStart(amends.deletedInsertList, startDom, startOffset);
            }
        }
        var end = {};
        if (endDom === startDom && !!start) {
            end.dom = start.dom;
            end.offset = endOffset;
        } else {
            end = checkEnd(amends.deleteList, endDom, endOffset);
            if (!end) {
                end = checkEnd(amends.insertList, endDom, endOffset);
                if (!end) {
                    end = checkEnd(amends.deletedInsertList, endDom, endOffset);
                }
            }
        }

        amends.start = start;
        amends.end = end;

        return amends;

        function checkStart(list, startDom, startOffset) {
            if (list.length === 0 || startOffset === 0) {
                return null;
            }
            var s = list[0];
            if (s === startDom || domUtils.contains(s, startDom)) {
                list.splice(0, 1);
                return {
                    dom: startDom,
                    offset: startOffset
                }
            }
            return null;
        }

        function checkEnd(list, endDom, endOffset) {
            if (list.length === 0) {
                return null;
            }
            var maxLength = (endDom.nodeType === 3 ? endDom.length : endDom.childNodes.length);
            if (endOffset === maxLength) {
                return null;
            }
            var e = list[list.length - 1];
            if (e === endDom || domUtils.contains(e, endDom)) {
                list.splice(list.length - 1, 1);
                return {
                    dom: endDom,
                    offset: endOffset
                }
            }
            return null;
        }
    },
    /**
     * 获取 wiz 编辑操作中 已标注的 Img 父节点
     * @param dom
     * @returns {*}
     */
    getWizAmendImgParent: function (dom) {
        return domUtils.getParentByFilter(dom, function (node) {
            return (node && node.nodeType === 1 &&
            node.getAttribute(CONST.ATTR.IMG));
        }, true);
    },
    /**
     * 获取 wiz 编辑操作中 已标注为编辑的 父节点
     * @param dom
     * @returns {*}
     */
    getWizAmendParent: function (dom) {
        return domUtils.getParentByFilter(dom, function (node) {
            return (node && node.nodeType === 1 &&
            (node.getAttribute(CONST.ATTR.SPAN_INSERT) || node.getAttribute(CONST.ATTR.SPAN_DELETE)));
        }, true);
    },
    /**
     * 获取 wiz 编辑操作中 已标注删除的 父节点
     * @param dom
     * @returns {*}
     */
    getWizDeleteParent: function (dom) {
        return domUtils.getParentByFilter(dom, function (node) {
            return (node && node.nodeType === 1 && node.getAttribute(CONST.ATTR.SPAN_DELETE));
        }, true);
    },
    /**
     * 获取 wiz 编辑操作中 已标注新增的 父节点
     * @param dom
     * @returns {*}
     */
    getWizInsertParent: function (dom) {
        return domUtils.getParentByFilter(dom, function (node) {
            //node.childNodes.length == 0 时，键盘敲入的字符加在 span 外面
            return (node && node.nodeType === 1 && node.getAttribute(CONST.ATTR.SPAN_INSERT) && !node.getAttribute(CONST.ATTR.SPAN_DELETE) && node.childNodes.length > 0);
        }, true);
    },
    /**
     * 获取 鼠标选择范围内（Range）满足条件的 Wiz Span
     * @param isAll
     * @param options
     * @returns {*}
     */
    getWizSpanFromRange: function (isAll, options) {
        var exp = 'span', i, j, d;
        if (!options) {
            return [];
        }
        //根据 options 生成 dom 查询表达式
        for (i in options) {
            if (options.hasOwnProperty(i)) {
                if (options[i]) {
                    exp += '[' + i + '="' + options[i] + '"]';
                } else {
                    exp += '[' + i + ']';
                }

            }
        }

        var sel = ENV.doc.getSelection(), range,
            startDom, startOffset,
            endDom, endOffset,
            startSpan, endSpan, parent, domList,
            startIndex, endIndex,
            dIdx, result = [];

        if (isAll) {
            //在 document.body 内进行查找
            var tmp = ENV.doc.querySelectorAll(exp);
            for (i = 0, j = tmp.length; i < j; i++) {
                result.push(tmp[i]);
            }
            return result;
        }

        if (sel.rangeCount === 0) {
            return [];
        }

        if (sel.isCollapsed) {
            endDom = rangeUtils.getRangeAnchor(false);
            startDom = domUtils.getPreviousNode(endDom, false, null);

            if (endDom) {
                endDom = domUtils.getParentByFilter(endDom, spanFilter, true);
                if (endDom) {
                    result.push(endDom);
                }
            }

            //TODO 对于换行的处理有问题，需要待定，暂时屏蔽
            //if (!endDom && startDom) {
            //    startDom = domUtils.getParentByFilter(startDom, spanFilter, true);
            //    if (startDom) {
            //        result.push(startDom);
            //    }
            //}

            return result;
        }
        startDom = rangeUtils.getRangeAnchor(true);
        endDom = rangeUtils.getRangeAnchor(false);

        if (!startDom || !endDom) {
            return [];
        }

        //获取 startDom, endDom 所在的 WizSpan 节点
        startSpan = domUtils.getParentByFilter(startDom, spanFilter, true);
        endSpan = domUtils.getParentByFilter(endDom, spanFilter, true);
        if (startSpan && startSpan == endSpan) {
            //startDom 和 endDom 所在同一个 WizSpan
            return [startSpan];
        }

        //在 startDom, endDom 共同的 parent 内根据查询表达式 查找 WizSpan
        parent = domUtils.getParentRoot([startDom, endDom]);
        domList = parent.querySelectorAll(exp);
        startIndex = domUtils.getIndexList(startDom);
        endIndex = domUtils.getIndexList(endDom);
        //startDom 是 TextNode 时，其父节点的 index 肯定要小于 startDom， 所以必须强行加入
        if (startSpan) {
            result.push(startSpan);
        }
        //根据 起始节点的 index 数据筛选 在其范围内的 WizSpan
        for (i = 0, j = domList.length; i < j; i++) {
            d = domList[i];
            dIdx = domUtils.getIndexList(d);
            if (domUtils.compareIndexList(startIndex, dIdx) <= 0 && domUtils.compareIndexList(endIndex, dIdx) >= 0) {
                result.push(d);
            }
        }
        return result;

        /**
         * 查找 attribute 满足 options 的 Dom 节点过滤器
         * @param node
         * @returns {boolean}
         */
        function spanFilter(node) {
            if (!node || node.nodeType !== 1) {
                return false;
            }
            var i;
            for (i in options) {
                //option[i] == '' 表示 只看某属性是否存在，但不比较具体的value
                if (options.hasOwnProperty(i) &&
                    ( !node.getAttribute(i) || (options[i] && node.getAttribute(i) != options[i]))
                ) {
                    return false;
                }
            }
            return true;
        }
    },
    /**
     * 判断 是否为修订编辑的 笔记
     */
    isAmendEdited: function () {
        var amendDoms = amendUtils.getAmendDoms({
            selection: true,
            selectAll: true
        });
        return !!amendDoms && ((amendDoms.deleteList.length > 0) ||
            (amendDoms.insertList.length > 0) ||
            (amendDoms.deletedInsertList.length > 0));
    },
    /**
     * 判断 是否为 修订的 dom
     * @param dom
     * @returns {*|boolean}
     */
    isWizAmend: function (dom) {
        return amendUtils.getWizAmendParent(dom);
    },
    /**
     * 判断 是否为 删除内容
     * @param dom
     * @returns {boolean}
     */
    isWizDelete: function (dom) {
        return !!amendUtils.getWizDeleteParent(dom);
    },
    /**
     * 判断 是否为 新增内容
     * @param dom
     * @returns {boolean}
     */
    isWizInsert: function (dom) {
        return !!amendUtils.getWizInsertParent(dom);
    }
};

module.exports = amendUtils;
},{"../../common/const":18,"../../common/env":20,"../../common/utils":24,"../../domUtils/domBase":28,"../../rangeUtils/rangeBase":48}],11:[function(require,module,exports){
/**
 * amend 中通用的基本方法集合（扩展操作）
 *
 */

var ENV = require('../../common/env'),
    CONST = require('../../common/const'),
    utils = require('../../common/utils'),
    domUtils = require('../../domUtils/domExtend'),
    commandExtend = require('../../editor/commandExtend'),
    rangeUtils = require('../../rangeUtils/rangeExtend'),
    amendUser = require('./../amendUser'),
    amendUtils = require('./amendBase');

/**
 * 添加 dom 到 getSelectedAmendDoms 或 getAmendDoms 方法得到的数据
 * @param amendDoms
 * @param dom
 */
amendUtils.add2SelectedAmendDoms = function (amendDoms, dom) {
    if (!dom) {
        return;
    }
    if (dom.getAttribute(CONST.ATTR.SPAN_INSERT) && dom.getAttribute(CONST.ATTR.SPAN_DELETE)) {
        amendDoms.deletedInsertList.push(dom);
    } else if (dom.getAttribute(CONST.ATTR.SPAN_INSERT)) {
        amendDoms.insertList.push(dom);
    } else if (dom.getAttribute(CONST.ATTR.SPAN_DELETE)) {
        amendDoms.deleteList.push(dom);
    }
};
/**
 * 创建用于 修订内容中 封装 img 的 span
 * @param type
 * @param user
 * @returns {HTMLElement}
 */
amendUtils.createDomForImg = function (type, user) {
    var tmp = ENV.doc.createElement('span');
    amendUtils.setDefaultAttr(tmp, user);
    tmp.setAttribute(CONST.ATTR.IMG, '1');
    if (type == CONST.TYPE.IMG_DELETE) {
        tmp.removeAttribute(CONST.ATTR.SPAN_INSERT);
        tmp.setAttribute(CONST.ATTR.SPAN_DELETE, user.hash);
    }
    amendUtils.setUserImgContainerStyle(tmp);

    return tmp;
};
/**
 * 创建用于 新建内容的 span
 * @param user
 * @returns {HTMLElement}
 */
amendUtils.createDomForInsert = function (user) {
    var tmp = ENV.doc.createElement('span');
    amendUtils.setDefaultAttr(tmp, user);
    amendUtils.setUserInsertStyle(tmp, user);
    tmp.innerHTML = CONST.FILL_CHAR;
    return tmp;
};
/**
 * 创建用于 反转修订时 新建内容的 span
 * @returns {HTMLElement}
 */
amendUtils.createDomForReverse = function () {
    var tmp = ENV.doc.createElement('span');
    tmp.innerHTML = CONST.FILL_CHAR;
    return tmp;
};
/**
 * 创建 用于粘贴的 span
 * @param id
 * @param user
 * @returns {{start: HTMLElement, content: HTMLElement, end: HTMLElement}}
 */
amendUtils.createDomForPaste = function (id) {
    var start, content, end;
    start = domUtils.createSpan();

    start.setAttribute(CONST.ATTR.SPAN_PASTE_TYPE, CONST.TYPE.PASTE.START);
    start.setAttribute(CONST.ATTR.SPAN_PASTE_ID, id);
    start.innerHTML = CONST.FILL_CHAR;

    content = domUtils.createSpan();
    content.setAttribute(CONST.ATTR.SPAN_PASTE_TYPE, CONST.TYPE.PASTE.CONTENT);
    content.setAttribute(CONST.ATTR.SPAN_PASTE_ID, id);
    content.innerHTML = CONST.FILL_CHAR + CONST.FILL_CHAR;

    end = domUtils.createSpan();
    end.setAttribute(CONST.ATTR.SPAN_PASTE_TYPE, CONST.TYPE.PASTE.END);
    end.setAttribute(CONST.ATTR.SPAN_PASTE_ID, id);
    end.innerHTML = CONST.FILL_CHAR;
    return {
        start: start,
        content: content,
        end: end
    };
};
/**
 * 标记 删除的 img
 * @param img
 * @param user
 */
amendUtils.deleteImg = function (img, user) {
    //必须首先判断 img 是否为已已标记修订的 img span 内
    var imgSpan = amendUtils.getWizAmendImgParent(img), mask;
    if (imgSpan) {
        //如果是已删除的， 则直接忽略
        //如果不是，则直接给 img span 添加 删除标识
        if (!imgSpan.getAttribute(CONST.ATTR.SPAN_DELETE)) {
            imgSpan.setAttribute(CONST.ATTR.SPAN_USERID, user.hash);
            imgSpan.setAttribute(CONST.ATTR.SPAN_DELETE, user.hash);
            mask = imgSpan.querySelector('img[' + CONST.ATTR.IMG_MASK + ']');
            domUtils.css(mask, CONST.CSS.IMG.MASK);
            domUtils.css(mask, CONST.CSS.IMG_DELETED);
        }
        return;
    }

    //因为 删除img 时，是在 img 外面封装span ，破坏了 range 的范围，
    // 所以如果 img 是在 range 的边缘时必须要修正
    var rangeEdge = rangeUtils.isRangeEdge(img);

    var nSpan = amendUtils.packageImg(img, CONST.TYPE.IMG_DELETE, user);

    if (rangeEdge.isStart) {
        rangeEdge.startDom = nSpan;
        rangeEdge.startOffset = 0;
    }
    if (rangeEdge.isEnd) {
        rangeEdge.endDom = nSpan.parentNode;
        rangeEdge.endOffset = domUtils.getIndex(nSpan) + 1;
    }

    if (rangeEdge.isCollapsed && rangeEdge.isStart) {
        rangeUtils.setRange(rangeEdge.startDom, domUtils.getEndOffset(rangeEdge.startDom), null, null);
    } else if (!rangeEdge.isCollapsed && (rangeEdge.isStart || rangeEdge.isEnd)) {
        rangeUtils.setRange(rangeEdge.startDom, rangeEdge.startOffset, rangeEdge.endDom, rangeEdge.endOffset);
    }
};
/**
 * 修正 删除图片操作后的 光标位置（用于修订）
 */
amendUtils.fixSelectionByDeleteImg = function () {
    var sel = ENV.doc.getSelection(),
        range = sel.getRangeAt(0),
        startDom, endDom, startOffset, endOffset,
        isDeleteImgStart = false,
        isDeleteImgEnd = false;

    if (sel.rangeCount === 0) {
        return;
    }

    //判断 startDom 是否处于已删除的 img span 内
    startDom = amendUtils.getWizAmendImgParent(range.startContainer);
    if (startDom && !startDom.getAttribute(CONST.ATTR.SPAN_DELETE)) {
        startDom = null;
    }
    if (!startDom) {
        startDom = range.startContainer;
        startOffset = range.startOffset;
    } else {
        isDeleteImgStart = true;
        startOffset = 0;
    }

    if (!sel.isCollapsed) {
        endDom = amendUtils.getWizAmendImgParent(range.endContainer);
        if (!endDom) {
            endDom = range.endContainer;
            endOffset = range.endOffset;
        } else {
            isDeleteImgEnd = true;
        }
    } else {
        endDom = startDom;
        isDeleteImgEnd = isDeleteImgStart;
    }

    if (isDeleteImgEnd && endDom && endDom.nextSibling) {
        endOffset = 0;
        endDom = endDom.nextSibling;
    } else if (isDeleteImgEnd && endDom) {
        endOffset = domUtils.getIndex(endDom) + 1;
        endDom = endDom.parentNode;
    } else {
        endOffset = range.endOffset;
    }

    if (sel.isCollapsed) {
        sel.collapse(endDom, endOffset);
    } else {
        sel.collapse(startDom, startOffset);
        sel.extend(endDom, endOffset);
    }

};
/**
 * 粘贴后，修改新粘贴的内容样式（设置为当前用户新建内容）
 * @param nSpanStart
 * @param nSpanEnd
 * @param user
 */
amendUtils.modifyDomForPaste = function (nSpanStart, nSpanEnd, user) {
    if (!nSpanStart || !nSpanEnd) {
        return;
    }

    if (nSpanStart.childNodes.length === 1 && nSpanStart.innerText == CONST.FILL_CHAR) {
        nSpanStart.innerHTML = '';
    }
    if (nSpanEnd.childNodes.length === 1 && nSpanEnd.innerText == CONST.FILL_CHAR) {
        nSpanEnd.innerHTML = '';
    }

    var parent = domUtils.getParentRoot([nSpanStart, nSpanEnd]);
    if (!parent) {
        return;
    }

    var tmpP, tmpD, tmpWizAmend,
        i, j, d,
        domResult, domList;

    domResult = domUtils.getListA2B({
        startDom: nSpanStart,
        startOffset: 0,
        endDom: nSpanEnd,
        endOffset: domUtils.getEndOffset(nSpanEnd)
    });
    domList = domResult.list;
    for (i = 0, j = domList.length; i < j; i++) {
        d = domList[i];
        tmpP = d.parentNode;
        tmpWizAmend = amendUtils.getWizAmendParent(d);
        if (!tmpP) {
            continue;
        }
        if (tmpWizAmend) {
            //如果是复制的 修订span ，则直接修改 span 为当前粘贴的用户
            d = tmpWizAmend;

        } else if (d.nodeType == 3) {
            if (utils.isEmpty(d.nodeValue)) {
                continue;
            }
            //粘贴操作后， 如果 PASTE_TYPE = CONTENT 的 span 内有 nodeType != 3 的节点，则不能直接修改 CONTENT 这个 span
            if (domUtils.isWizSpan(tmpP) && tmpP.children.length === 0) {
                d = tmpP;
            } else {
                tmpD = amendUtils.createDomForInsert(user);
                tmpD.innerHTML = '';
                tmpP.insertBefore(tmpD, d);
                tmpD.appendChild(d);
                d = tmpD;
            }
        }

        if (domUtils.isTag(d, 'img')) {
            d = amendUtils.packageImg(d, CONST.TYPE.IMG_INSERT, user);
        } else if (domUtils.isSelfClosingTag(d)) {
            continue;
        }
        amendUtils.setDefaultAttr(d, user);
        amendUtils.setUserInsertStyle(d, user);
    }

    //清理空的临时 span
    if (parent != ENV.doc.body && parent != ENV.doc.body.parentNode && parent.parentNode) {
        parent = parent.parentNode;
    }
    domList = parent.querySelectorAll('span[' + CONST.ATTR.SPAN_PASTE_TYPE + ']');
    for (i = 0, j = domList.length; i < j; i++) {
        d = domList[i];
        if (d.childNodes.length === 0) {
            domUtils.remove(d);
        } else {
            d.removeAttribute(CONST.ATTR.SPAN_PASTE_TYPE);
            d.removeAttribute(CONST.ATTR.SPAN_PASTE_ID);
        }
    }
};
/**
 * 封装修订中的 img（用于删除、粘贴的图片）
 * @param img
 * @param type
 * @param user
 * @returns {HTMLElement}
 */
amendUtils.packageImg = function (img, type, user) {
    //添加元素的顺序不要随便改动， 会影响 selection 光标的位置
    var pNode, nextNode, tmpNode,
        nSpan = amendUtils.createDomForImg(type, user);
    pNode = img.parentNode;
    nextNode = img.nextSibling;
    while (nextNode && nextNode.nodeType == 3 && nextNode.nodeValue == CONST.FILL_CHAR) {
        tmpNode = nextNode;
        nextNode = nextNode.nextSibiling;
        domUtils.remove(tmpNode);
    }
    nSpan.appendChild(img);
    //添加遮罩
    var mask = ENV.doc.createElement('img');
    mask.className += CONST.CLASS.IMG_NOT_DRAG;
    mask.setAttribute(CONST.ATTR.IMG_MASK, '1');
    //手机客户端有的情况下会设置 img max-width = 80%
    if (img.style.maxWidth) {
        mask.style.maxWidth = img.style.maxWidth;
    }
    if (img.style.maxHeight) {
        mask.style.maxHeight = img.style.maxHeight;
    }
    if (img.style.width) {
        mask.style.width = img.style.width;
    }
    if (img.style.height) {
        mask.style.height = img.style.height;
    }
    domUtils.css(mask, CONST.CSS.IMG.MASK);
    if (type == CONST.TYPE.IMG_DELETE) {
        domUtils.css(mask, CONST.CSS.IMG_DELETED);
    } else {
        domUtils.css(mask, CONST.CSS.IMG_INSERT);
    }
    nSpan.appendChild(mask);
    pNode.insertBefore(nSpan, nextNode);
    return nSpan;
};
/**
 * 删除 有当前用户删除标记的内容
 * @param parentRoot
 * @param user
 */
amendUtils.removeUserDel = function (parentRoot, user) {
    var deleteDomList = [], i, j, dom, p;
    if (!parentRoot) {
        parentRoot = rangeUtils.getRangeParentRoot();
    }
    if (parentRoot) {
        if (!domUtils.isBody(parentRoot)) {
            //避免直接返回最底层的 span，从而导致查询失败，所以需要扩大范围
            parentRoot = parentRoot.parentNode;
        }
        //判断当前的 元素是否是在 已封包的 修订 img 中
        dom = amendUtils.getWizAmendImgParent(parentRoot);

        //只获取当前用户修订的 img span
        if (dom && dom.getAttribute(CONST.ATTR.SPAN_USERID) !== user.hash) {
            dom = null;
        }

        if (dom) {
            // 针对 img 特殊处理
            deleteDomList.push(dom);
        } else {
            domUtils.search(parentRoot, '[' +
                CONST.ATTR.SPAN_INSERT + '="' + user.hash + '"][' +
                CONST.ATTR.SPAN_DELETE + '="' + user.hash + '"]', deleteDomList);

            //TODO 此种情况可能已经不会存在了
            domUtils.search(parentRoot, '[' +
                CONST.ATTR.SPAN_USERID + '="' + user.hash + '"] [' +
                CONST.ATTR.SPAN_DELETE + '="' + user.hash + '"]', deleteDomList);
        }
    }

    for (i = 0, j = deleteDomList.length; i < j; i++) {
        dom = deleteDomList[i];
        p = dom.parentNode;
        p.removeChild(dom);
        domUtils.removeEmptyParent(p);
    }
};
/**
 * 根据 user 获取删除操作时，需要设置的 style & attr
 * @param user
 * @returns {{attr: {}, style: {color: *, text-decoration: string}}}
 */
amendUtils.getDeletedStyle = function (user) {
    var attr = {};
    attr[CONST.ATTR.SPAN_DELETE] = user.hash;
    attr[CONST.ATTR.SPAN_USERID] = user.hash;
    attr[CONST.ATTR.SPAN_TIMESTAMP] = utils.getTime();

    var style = {'color': user.color, 'text-decoration': 'line-through'};

    return {
        attr: attr,
        style: style
    };
};
/**
 * 对光标选择范围设置 当前用户的 删除标记
 * @param user
 */
amendUtils.removeSelection = function (user) {
    var sel = ENV.doc.getSelection(),
        range = sel.getRangeAt(0),
        startDom = range.startContainer,
        startOffset = range.startOffset,
        endDom = range.endContainer,
        endOffset = range.endOffset,
        startImg = amendUtils.getWizAmendImgParent(startDom),
        endImg = amendUtils.getWizAmendImgParent(endDom),
        splitInsert;

    //如果开始 或结尾 是 修订的内容，但不是 img 的时候， 需要进行拆分后处理
    if (!endImg) {
        splitInsert = amendUtils.splitInsertDom(endDom, endOffset, true, user);
        if (splitInsert.isInsert && splitInsert.split) {
            rangeUtils.setRange(startDom, startOffset, endDom, endOffset);
        }
    }
    if (!startImg) {
        splitInsert = amendUtils.splitInsertDom(startDom, startOffset, true, user);
        if (splitInsert.isInsert && splitInsert.split) {
            //如果 选中的是 某一个dom 的中间部分
            if (endDom === startDom) {
                endDom = splitInsert.insertDom.nextSibling;
                endOffset = endDom.childNodes.length;
            }
            startDom = splitInsert.insertDom;
            startOffset = splitInsert.insertDom.childNodes.length;
            rangeUtils.setRange(startDom, startOffset, endDom, endOffset);
        }
    }

    if (sel.isCollapsed) {
        //如果扩展范围后，依然为 折叠状态， 不进行任何删除样式的修改
        return;
    }

    var style = amendUtils.getDeletedStyle(user);
    rangeUtils.modifySelectionDom(
        style.style,
        style.attr
    );
    amendUtils.fixSelectionByDeleteImg();
};
/**
 * 初始化 新增span 的属性
 * @param dom
 * @param user
 */
amendUtils.setDefaultAttr = function (dom, user) {
    if (dom.nodeType == 1) {
        dom.setAttribute(CONST.ATTR.SPAN, CONST.ATTR.SPAN);
        dom.setAttribute(CONST.ATTR.SPAN_INSERT, user.hash);
        dom.setAttribute(CONST.ATTR.SPAN_USERID, user.hash);
        dom.setAttribute(CONST.ATTR.SPAN_TIMESTAMP, utils.getTime());
    }
};
/**
 * 初始化已删除图片外层 span 的修订样式
 * @param dom
 */
amendUtils.setUserImgContainerStyle = function (dom) {
    domUtils.css(dom, CONST.CSS.IMG.SPAN);
};
/**
 * 初始化用户修订样式
 * @param dom
 * @param user
 */
amendUtils.setUserInsertStyle = function (dom, user) {
    domUtils.css(dom, {
        'color': user.color,
        'text-decoration': 'underline'
    });
};

/**
 * 根据 修订Dom 处理 Range 范围 （主要是 img 处于 Range 边缘时）
 * @returns {{startImg: *, endImg: *, startDom: Node, startOffset: Number, endDom: Node, endOffset: Number, leftDom: *, rightDom: *}}
 */
amendUtils.fixedAmendRange = function () {
    var sel = ENV.doc.getSelection(),
        range = sel.getRangeAt(0),
        startDom = range.startContainer,
        endDom = range.endContainer,
        startOffset = range.startOffset,
        endOffset = range.endOffset;

    //判断光标范围， 光标编辑区边界如果是 修订的 img 必须要把 img 全部选中
    var leftDom, rightDom, startInnerDom, endInnerDom,
        startImg, endImg;
    if (sel.isCollapsed) {
        rightDom = rangeUtils.getRangeAnchor(false);
        //如果光标在某个 textNode 中间， 则前后都是当前这个 textNode
        if (endDom.nodeType === 3 && endOffset > 0 && endOffset < endDom.nodeValue.length) {
            leftDom = rightDom;
        } else {
            leftDom = domUtils.getPreviousNode(rightDom, false, null);
        }

    } else {
        startInnerDom = rangeUtils.getRangeAnchor(true);
        endInnerDom = rangeUtils.getRangeAnchor(false);
        startImg = amendUtils.getWizAmendImgParent(startInnerDom);
        endImg = amendUtils.getWizAmendImgParent(endInnerDom);

        if (startImg) {
            startDom = startImg;
            startOffset = 0;
        }
        if (endImg) {
            endDom = endImg;
            endOffset = endImg.childNodes.length;
        }
        if (startImg || endImg) {
            rangeUtils.setRange(startDom, startOffset, endDom, endOffset);
        }
    }

    return {
        startImg: startImg,
        endImg: endImg,
        startDom: startDom,
        startOffset: startOffset,
        endDom: endDom,
        endOffset: endOffset,
        leftDom: leftDom,
        rightDom: rightDom
    }
};
/**
 * 根据 range 拆分 amend span （主要用于 普通编辑 & 在 amend span 内添加其他 html）
 * @param  fixed (amendUtils.fixedAmendRange 方法的返回值)
 */
amendUtils.splitAmendDomByRange = function (fixed) {
    var sel = ENV.doc.getSelection(),
        range,
        startDom = fixed.startContainer,
        endDom = fixed.endContainer,
        startOffset = fixed.startOffset,
        endOffset = fixed.endOffset,
        startImg, endImg;

    if (!sel.isCollapsed) {
        commandExtend.execCommand('delete');
        range = sel.getRangeAt(0);
        endDom = range.endContainer;
        endOffset = range.endOffset;
    } else {
        startImg = amendUtils.getWizAmendImgParent(fixed.leftDom);
        endImg = amendUtils.getWizAmendImgParent(fixed.rightDom);
        if (endImg) {
            endDom = endImg;
            endOffset = 0;
            rangeUtils.setRange(endDom, endOffset, endDom, endOffset);
        } else if (startImg) {
            startDom = startImg;
            startOffset = startImg.childNodes.length;
            rangeUtils.setRange(startDom, startOffset, startDom, startOffset);
        }

        range = sel.getRangeAt(0);
        endDom = range.endContainer;
        endOffset = range.endOffset;
    }

    var newDom = amendUtils.splitAmendDomForReverse(endDom, endOffset);
    if (newDom) {
        rangeUtils.setRange(newDom, 1, newDom, 1);
        return newDom;
    }
    return null;
};
/**
 * 如果在 已删除的文字内，需要筛分 已删除Dom，在中间添加
 * @param endDom
 * @param endOffset
 * @returns {boolean}
 */
amendUtils.splitDeletedDom = function (endDom, endOffset) {
    if (endDom.nodeType == 1) {
        return false;
    }
    var splitDom = null;
    if (amendUtils.isWizDelete(endDom)) {
        splitDom = amendUtils.splitWizDomWithTextNode(endDom, endOffset);
        return !!splitDom;
    }
    return false;
};
/**
 * 如果在 wiz span 内，进行操作时 需要拆分 Dom，在中间添加
 * @param endDom
 * @param endOffset
 * @param forceSplit
 * @param user
 * @returns {{}}
 */
amendUtils.splitInsertDom = function (endDom, endOffset, forceSplit, user) {
    var result = {
        insertDom: null,
        isInsert: false,
        split: false
    };
    if (!endDom) {
        return result;
    }
    if (endDom.nodeType == 1 && endOffset > 0) {
        endDom = endDom.childNodes[endOffset - 1];
        endOffset = domUtils.getEndOffset(endDom);
    } else if (endDom.nodeType == 1) {
        endDom = endDom.childNodes[0];
    }
    if (!endDom) {
        return result;
    }
    var imgDom = amendUtils.getWizAmendImgParent(endDom),
        insertDom = amendUtils.getWizInsertParent(endDom), time1, time2;
    result.insertDom = insertDom;
    if (!insertDom && endDom.nodeType == 1) {
        return result;
    }
    if (imgDom) {
        return result;
    }

    if (insertDom &&
        (forceSplit || insertDom.getAttribute(CONST.ATTR.SPAN_USERID) !== user.hash)) {
        //强迫分割（粘贴操作、Enter）时，直接分隔，不考虑时间
        result.split = true;
    } else if (insertDom) {
        //对于同一个用户，新增内容的在 AMEND_TIME_SPACE 时间间隔内，则仅更新时间戳，否则拆分 span
        time1 = insertDom.getAttribute(CONST.ATTR.SPAN_TIMESTAMP);
        time2 = utils.getTime();
        if (utils.getDateForTimeStr(time2) - utils.getDateForTimeStr(time1) >= CONST.AMEND_TIME_SPACE) {
            result.split = true;
        } else {
            insertDom.setAttribute(CONST.ATTR.SPAN_TIMESTAMP, time2);
        }
    }

    if (result.split) {
        result.split = !!amendUtils.splitWizDomWithTextNode(endDom, endOffset);
    }

    result.isInsert = !!insertDom;
    return result;
};
/**
 * 如果在 已修订的span 内，进行操作时 需要拆分 Dom，避免修订样式被继承（专门用于 逆修订）
 * @param endDom
 * @param endOffset
 * @returns {}
 */
amendUtils.splitAmendDomForReverse = function (endDom, endOffset) {
    var imgDom = amendUtils.getWizAmendImgParent(endDom);

    if (!imgDom && endDom.nodeType == 1 && endOffset > 0) {
        endDom = endDom.childNodes[endOffset - 1];
        endOffset = domUtils.getEndOffset(endDom);
    } else if (!imgDom && endDom.nodeType == 1) {
        endDom = endDom.childNodes[0];
    }
    if (!endDom) {
        return null;
    }
    var insertDom = amendUtils.getWizInsertParent(endDom),
        deleteDom = amendUtils.getWizDeleteParent(endDom),
        amendDom = insertDom || deleteDom,
        newDom = amendUtils.createDomForReverse();

    if (imgDom) {
        domUtils.before(newDom, imgDom, endOffset > 0);
    } else if (amendDom) {
        amendDom = amendUtils.splitWizDomWithTextNode(endDom, endOffset);
        if (amendDom) {
            domUtils.after(newDom, amendDom);
        } else {
            return null;
        }

    } else {
        return null;
    }

    return newDom;
};
/**
 * 把 根据 getSelectedAmendDoms 或 getAmendDoms 方法得到的数据中 起始、结束位置的 dom 进行拆分， 实现选择范围内 接受、拒绝修订
 * @param amendDoms
 */
amendUtils.splitSelectedAmendDoms = function (amendDoms) {
    if (!amendDoms || (!amendDoms.start && !amendDoms.end)) {
        return;
    }

    var sel = ENV.doc.getSelection(),
        range = sel.getRangeAt(0),
        startDom = range.startContainer,
        startOffset = range.startOffset,
        endDom = range.endContainer,
        endOffset = range.endOffset;

    var node;

    if (amendDoms.start && amendDoms.end && amendDoms.start.dom == amendDoms.end.dom) {
        //选择范围在 一个 dom 内部的时候，会把一个 dom 拆分为 3 段
        //为保证主 dom 不丢失，所以一定要先 end 后 start
        amendUtils.splitWizDomWithTextNode(amendDoms.end.dom, amendDoms.end.offset);
        node = amendUtils.splitWizDomWithTextNode(amendDoms.start.dom, amendDoms.start.offset);
        node = node.nextSibling;
        amendUtils.add2SelectedAmendDoms(amendDoms, node);
        startDom = node;
        startOffset = 0;
        endDom = node;
        endOffset = node.childNodes.length;
    } else {
        //单独拆分 选择范围的起始dom 和 结束dom
        if (amendDoms.start) {
            node = amendUtils.splitWizDomWithTextNode(amendDoms.start.dom, amendDoms.start.offset);
            node = node.nextSibling;
            amendUtils.add2SelectedAmendDoms(amendDoms, node);
            startDom = node;
            startOffset = 0;
        }
        if (amendDoms.end) {
            node = amendUtils.splitWizDomWithTextNode(amendDoms.end.dom, amendDoms.end.offset);
            amendUtils.add2SelectedAmendDoms(amendDoms, node);
            endDom = node;
            endOffset = node.childNodes.length;
        }
    }
    delete amendDoms.start;
    delete amendDoms.end;
    //修正选择范围
    rangeUtils.setRange(startDom, startOffset, endDom, endOffset)
};
/**
 * 从 TextNode 的 光标位置 拆分该 TextNode 的 修订 Dom
 * @param endDom
 * @param endOffset
 * @returns {*}  //返回最后拆分的 Dom
 */
amendUtils.splitWizDomWithTextNode = function (endDom, endOffset) {
    if (!endDom || endDom.nodeType !== 3) {
        return null;
    }
    var tmpSplitStr, tmpSplit, tmpParent, tmpDom,
        lastSplit = null;
    if (endOffset < endDom.nodeValue.length) {
        tmpSplitStr = endDom.nodeValue.substr(endOffset);
        tmpSplit = endDom.cloneNode(false);
        tmpSplit.nodeValue = tmpSplitStr;
        endDom.nodeValue = endDom.nodeValue.substr(0, endOffset);
        endDom.parentNode.insertBefore(tmpSplit, endDom.nextSibling);
        lastSplit = endDom;
        tmpParent = endDom.parentNode;
        tmpDom = tmpSplit;
    } else {
        tmpParent = endDom.parentNode;
        tmpDom = endDom.nextSibling;
    }
    while (!!tmpParent && !domUtils.isBody(tmpParent)) {
        lastSplit = tmpParent;
        domUtils.splitDomBeforeSub(tmpParent, tmpDom);
        if (tmpParent && tmpParent.nodeType === 1 &&
            (tmpParent.getAttribute(CONST.ATTR.SPAN_DELETE) ||
            tmpParent.getAttribute(CONST.ATTR.SPAN_INSERT))) {
            break;
        }
        tmpDom = tmpParent.nextSibling;
        tmpParent = tmpParent.parentNode;

    }
    return lastSplit
};
/**
 * 删除 修订内容（接受 已删除的； 拒绝已添加的）
 * @param domList
 */
amendUtils.wizAmendDelete = function (domList) {
    var i, j, d, p;
    for (i = 0, j = domList.length; i < j; i++) {
        d = domList[i];
        p = d.parentNode;
        p.removeChild(d);
        domUtils.removeEmptyParent(p);
    }
};
/**
 * 保留 修订内容（接受 已添加的； 拒绝已删除的）
 * @param domList
 */
amendUtils.wizAmendSave = function (domList) {
    var i, j, d, u;
    for (i = 0, j = domList.length; i < j; i++) {
        d = domList[i];

        if (d.getAttribute(CONST.ATTR.SPAN_DELETE) && d.getAttribute(CONST.ATTR.SPAN_INSERT) &&
            d.getAttribute(CONST.ATTR.SPAN_INSERT) !== d.getAttribute(CONST.ATTR.SPAN_USERID)
        ) {
            //如果 是用户B 删除了 用户A 新增的内容， 则拒绝已删除操作时，恢复为用户A 新增的状态
            u = amendUser.getUserByGuid(d.getAttribute(CONST.ATTR.SPAN_INSERT));
            u = u ? u : {};
            d.removeAttribute(CONST.ATTR.SPAN_DELETE);
            d.setAttribute(CONST.ATTR.SPAN_USERID, u.hash);

            if (d.getAttribute(CONST.ATTR.IMG)) {
                amendUtils.setUserImgContainerStyle(d);
                domUtils.css(mask, CONST.CSS.IMG_INSERT);
            } else {
                amendUtils.setUserInsertStyle(d, u);
            }
            continue;
        }

        if (d.getAttribute(CONST.ATTR.IMG)) {
            domUtils.before(d.children[0], d);
            domUtils.remove(d);
        } else {
            domUtils.css(d, {
                'color': '',
                'text-decoration': ''
            });
//                    d.removeAttribute(CONST.ATTR.SPAN);
            d.removeAttribute(CONST.ATTR.SPAN_USERID);
            d.removeAttribute(CONST.ATTR.SPAN_INSERT);
            d.removeAttribute(CONST.ATTR.SPAN_DELETE);
            d.removeAttribute(CONST.ATTR.SPAN_PASTE);
            d.removeAttribute(CONST.ATTR.SPAN_PASTE_TYPE);
            d.removeAttribute(CONST.ATTR.SPAN_PASTE_ID);
            d.removeAttribute(CONST.ATTR.SPAN_TIMESTAMP);
        }
    }
};

module.exports = amendUtils;
},{"../../common/const":18,"../../common/env":20,"../../common/utils":24,"../../domUtils/domExtend":29,"../../editor/commandExtend":33,"../../rangeUtils/rangeExtend":49,"./../amendUser":9,"./amendBase":10}],12:[function(require,module,exports){
/**
 * 块级元素区域（Table & CodeMirror） 操作核心包 core
 * 1、处理 Table & CodeMirror 横向滚动条
 * 2、处理 块级元素前后自动添加 空行
 * 最新需求 CodeMirror 在移动设备上自动换行，不显示横向滚动条
 */

var ENV = require('../common/env'),
    CONST = require('../common/const'),
    codeUtils = require('../codeUtils/codeUtils'),
    domUtils = require('../domUtils/domExtend'),
    tableUtils = require('../tableUtils/tableUtils'),
    blockUtils = require('./blockUtils');

var BlockType = {
    Table: 'table',
    Code: 'code'
};
var scrollContainer, scroll;
var blockList = [];

var findTimer;
function findBlocksController() {
    if (scroll && scroll.parentNode !== scrollContainer) {
        // 滚动时 如果 自定义滚动条正在显示，则立即控制滚动条是否隐藏
        checkScroll();
    }

    if (findTimer) {
        clearTimeout(findTimer);
    }
    findTimer = setTimeout(findBlocks, 100);
}

function findBlocks() {
    blockList = [];
    var i;

    var scrollTarget;
    // 查找 table
    var _tableList = ENV.doc.querySelectorAll('table');
    var table, tableContainer;
    for (i = _tableList.length - 1; i >= 0; i--) {
        table = _tableList[i];
        tableContainer = tableUtils.getContainer(table);
        if (!tableContainer) {
            tableUtils.initTableContainer(table);
            tableContainer = tableUtils.getContainer(table);
        }
        scrollTarget = tableContainer.querySelector('.' + CONST.CLASS.TABLE_BODY);
        blockList.push({
            type: BlockType.Table,
            target: scrollTarget,
            top: tableContainer.offsetTop,
            height: tableContainer.offsetHeight
        });
        // console.log(tTop + tHeight > scrollTop + viewHeight);
    }

    // 查找 code
    // 非 手机端、Pad 端才处理 Code 的 横向滚动条
    if (!(ENV.client.type.isPhone || ENV.client.type.isPad)) {
        var _codeList = ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_MIRROR);
        var code, codeContainer;
        for (i = _codeList.length - 1; i >= 0; i--) {
            code = _codeList[i];
            codeContainer = codeUtils.getContainerFromChild(code);
            if (!codeContainer) {
                codeUtils.fixCode(code);
                codeContainer = codeUtils.getContainerFromChild(code);
            }
            if (!codeContainer) {
                continue;
            }
            // 不使用 codeMirror 自定义的滚动条
            scrollTarget = code.querySelector('.' + CONST.CLASS.CODE_MIRROR_HSCROLL);
            blockList.push({
                type: BlockType.Code,
                target: scrollTarget,
                codeMirror: codeContainer.codeMirror,
                top: codeContainer.offsetTop,
                height: codeContainer.offsetHeight
            });
            // console.log(tTop + tHeight > scrollTop + viewHeight);
        }
    }

    checkScroll();
}
function checkScroll() {
    if (blockList.length === 0) {
        return;
    }

    var viewHeight = ENV.doc.documentElement.clientHeight;
    var scrollTop = ENV.doc.body.scrollTop;
    var viewTop = scrollTop + viewHeight;
    var i, blockObj;

    for (i = blockList.length - 1; i >= 0; i--) {
        blockObj = blockList[i];
        if (blockObj.top < viewTop &&
            blockObj.top + blockObj.height > viewTop &&
            blockObj.target.offsetWidth < blockObj.target.scrollWidth) {
            if (scroll && scroll.curBlock && scroll.curBlock.target === blockObj.target) {
                // 如果
                setScrollSize(blockObj);
                return;
            } else if (scroll) {
                hideDiyScroll();
            }
            showDiyScroll(blockObj);
            return;
        }
    }
    // 没有任何一个需要显示时，则隐藏 scroll
    hideDiyScroll();
}

function hideDiyScroll() {
    if (scroll) {
        _event.unbindScroll();
        scroll.curBlock = null;
        scrollContainer.appendChild(scroll);
    }
}
function setScrollSize(blockObj) {
    var scrollInner;
    if (scroll) {
        scrollInner = scroll.children[0];
    } else {
        scroll = ENV.doc.createElement(CONST.TAG.TMP_TAG);
        scrollInner = ENV.doc.createElement('div');
        scroll.appendChild(scrollInner);
        domUtils.addClass(scroll, CONST.CLASS.BLOCK_SCROLL);
    }

    var targetWidth = blockObj.target.scrollWidth;
    domUtils.css(scrollInner, {
        "width": targetWidth + "px",
        "height": "1px",
        "background-color": "rgba(255, 255, 255, 0.2)",
    });

    domUtils.css(scroll, {
        "background-color": "rgba(255, 255, 255, 0.2)",
        "height": "17px",
        "width": blockObj.target.offsetWidth + "px",
        "overflow-x": "scroll",
        "overflow-y": "hidden",
        "position":"fixed",
        "bottom": "0px",
        "z-index": 999
    });

    scroll.scrollLeft = blockObj.target.scrollLeft;
}
function showDiyScroll(blockObj) {
    setScrollSize(blockObj);
    scroll.curBlock = blockObj;
    domUtils.after(scroll, blockObj.target);
    _event.bindScroll();
    // 显示自定义滚动条时，需要与 目标横向滚动条位置相同
    _event.handler.onTxScroll();

}

var _event = {
    bind: function() {
        _event.unbind();
        if (!ENV.client.type.isIOS && !ENV.client.type.isAndroid) {
            ENV.event.add(CONST.EVENT.ON_SCROLL, _event.handler.onScroll);
            ENV.event.add(CONST.EVENT.ON_DOM_SUBTREE_MODIFIED, _event.handler.updateRender);
            ENV.win.addEventListener('resize', _event.handler.updateRender);
        }
        if (!ENV.readonly) {
            if (ENV.client.type.isPhone || ENV.client.type.isPad) {
                ENV.event.add(CONST.EVENT.ON_TOUCH_START, _event.handler.onMouseDown);
            } else {
                ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
            }
        }
    },
    unbind: function() {
        hideDiyScroll();
        ENV.event.remove(CONST.EVENT.ON_SCROLL, _event.handler.onScroll);
        ENV.event.remove(CONST.EVENT.ON_DOM_SUBTREE_MODIFIED, _event.handler.updateRender);
        ENV.win.removeEventListener('resize', _event.handler.updateRender);

        ENV.event.remove(CONST.EVENT.ON_TOUCH_START, _event.handler.onMouseDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
    },
    bindScroll: function() {
        _event.unbindScroll();
        // 避免 滚动条循环事件导致 滚动不流畅， 不监听 target 的 scroll 事件
        // scroll.curBlock.target.addEventListener('scroll', _event.handler.onTxScroll);
        scroll.addEventListener('scroll', _event.handler.onSxScroll);
    },
    unbindScroll: function() {
        // if (scroll.curBlock) {
        //     scroll.curBlock.target.removeEventListener('scroll', _event.handler.onTxScroll);
        // }
        scroll.removeEventListener('scroll', _event.handler.onSxScroll);
    },
    handler: {
        onMouseDown: function (e) {
            var isLeft = (e.type !== 'mousedown' || e.button === 0 || e.button === 1);
            if (!isLeft) {
                return;
            }
            blockUtils.checkAndInsertEmptyLine(e);
        },
        onScroll: function () {
            findBlocksController();
        },
        onSxScroll: function () {
            if (scroll.curBlock) {
                scroll.curBlock.target.scrollLeft = scroll.scrollLeft;
            }
        },
        onTxScroll: function () {
            if (scroll.curBlock) {
                scroll.scrollLeft = scroll.curBlock.target.scrollLeft;
            }
        },
        updateRender: function () {
            findBlocksController();
        }
    }
};

var blockCore = {
    on: function() {
        _event.bind();

        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            return;
        }
        if (!scrollContainer) {
            scrollContainer = ENV.doc.createElement('div');
        }
        findBlocksController();
    },
    off: function() {
        _event.unbind();
    }
};

module.exports = blockCore;
},{"../codeUtils/codeUtils":16,"../common/const":18,"../common/env":20,"../domUtils/domExtend":29,"../tableUtils/tableUtils":54,"./blockUtils":13}],13:[function(require,module,exports){
/**
 * wiz 模块区域（表格、代码）操作的基本方法集合
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    domUtils = require('../domUtils/domExtend'),
    commandExtend = require('../editor/commandExtend'),
    tableZone = require('../tableUtils/tableZone'),
    rangeUtils = require('../rangeUtils/rangeExtend');

function checkPageFirst(checkFirst, clientY, getBlockFun) {
    var isFirst = false, isLast = false,
        offset, container;
    var obj = checkFirst ? domUtils.getFirstDeepChild(ENV.doc.body) : domUtils.getLastDeepChild(ENV.doc.body);
    container = domUtils.getParentByClass(obj, CONST.CLASS.CODE_CONTAINER);

    if (container) {
        isFirst = true;
    } else {
        while (obj && !domUtils.canEdit(obj)) {
            obj = checkFirst ? domUtils.getNextNodeCanEdit(obj, false) : domUtils.getPreviousNodeCanEdit(obj, false);
        }
    }

    if (obj) {
        if (obj.nodeType == 3) {
            obj = obj.parentNode;
        }
        offset = domUtils.getOffset(obj);
        container = domUtils.getParentByFilter(obj, getBlockFun, true);
        if (checkFirst && clientY < offset.top && container) {
            isFirst = true;
        } else if (!checkFirst && clientY > offset.top + obj.offsetHeight && container) {
            isLast = true;
        } else {
            obj = null;
        }
    }
    return {
        container: container,
        isFirst: isFirst,
        isLast: isLast
    }
}

function getBlockFun(obj) {
    return domUtils.hasClass(obj, CONST.CLASS.CODE_CONTAINER) ||
        domUtils.hasClass(obj, CONST.CLASS.TABLE_CONTAINER);
}

function getBlockContainer(obj) {
    return domUtils.getParentByFilter(obj, getBlockFun, true)
}

function isBlock(obj) {
    return domUtils.hasClass(obj, CONST.CLASS.CODE_CONTAINER) ||
        domUtils.hasClass(obj, CONST.CLASS.TABLE_CONTAINER) ||
        domUtils.hasClass(obj, CONST.CLASS.TABLE_BODY)
}

var blockUtils = {
    /**
     * 为防止笔记内只有一个模块区域（表格、代码等）时 无法在模块前后 或 多个模块之间输入内容
     */
    checkAndInsertEmptyLine: function (e) {
        var touch = e.changedTouches ? e.changedTouches[0] : null,
            target = touch ? touch.target : e.target,
            eventClient = utils.getEventClientPos(e),
            clientY = ENV.doc.body.scrollTop + eventClient.y;
        var dom, container,
            checkResult, isAfter = false, isBefore = false,
            offsetY;

        if (target === ENV.doc.body || target === ENV.doc.body.parentNode) {
            // 只有一个 CodeMirror 时，点击 CodeMirror 下面， range = null
            if (target === ENV.doc.body) {
                checkResult = checkPageFirst(true, clientY, getBlockFun);
                isBefore = checkResult.isFirst;
                container = checkResult.container;
            }
            if (!isBefore) {
                checkResult = checkPageFirst(false, clientY, getBlockFun);
                isAfter = checkResult.isLast;
                container = checkResult.container;
            }

        } else if (isBlock(target)) {
            container = getBlockContainer(target);
            offsetY = 0;
            if (container) {
                offsetY = clientY - domUtils.getOffset(container).top;
            }
            if (container && offsetY < 15) {
                dom = domUtils.getPreviousNodeCanEdit(target, false);
                if (!dom || getBlockContainer(dom)) {
                    isBefore = true;
                }
            } else if (container && (target.offsetHeight - offsetY) < 15) {
                dom = domUtils.getNextNodeCanEdit(target, false);
                if (!dom || getBlockContainer(dom)) {
                    isAfter = true;
                }
            }
        }
        if (container && (isAfter || isBefore)) {
            historyUtils.saveSnap(false);
            blockUtils.insertEmptyLine(container, isAfter);
        } else {
            dom = null;
        }
    },
    insertBlock: function(target) {
        var range = rangeUtils.getRange();
        var start, curDom, curBlock, br;

        if (range) {
            range = rangeUtils.getRange();
            start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            curDom = start.container;
            curBlock = domUtils.getBlockParent(curDom, true);
            if (domUtils.hasClass(curBlock, CONST.CLASS.TODO_LAYER)) {
                // 如果光标在 TodoList 内，则直接在 TodoList 下面创建
                curDom = ENV.doc.createElement(curBlock.tagName);
                domUtils.after(curDom, curBlock);
                curDom.appendChild(target);
                curBlock = curDom;
            } else if (!curBlock || !domUtils.isEmptyDom(curBlock)) {
                // 如果光标位置非空，先插入 段落
                commandExtend.execCommand('insertparagraph');
                range = rangeUtils.getRange();
                curDom = range.startContainer;
                curBlock = domUtils.getBlockParent(curDom, true);
            }

            if (domUtils.isEmptyDom(curBlock) && domUtils.isTag(curBlock, 'div')) {
                // 如果 光标位置 block 模块为空，直接注入 target
                curBlock.innerHTML = '';
                curBlock.appendChild(target);
            } else {
                // 如果 光标位置 block 模块非空，则在 block 之前插入 block，然后再注入 target
                curDom = ENV.doc.createElement('div');
                domUtils.before(curDom, curBlock);
                curDom.appendChild(target);
                if (domUtils.isEmptyDom(curBlock)) {
                    domUtils.remove(curBlock);
                }
                curBlock = curDom;
            }
        } else {
            curBlock = ENV.doc.createElement('div');
            curBlock.appendChild(target);
            ENV.doc.body.appendChild(curBlock);
            br = ENV.doc.createElement('div');
            br.appendChild(ENV.doc.createElement('br'));
            ENV.doc.body.appendChild(br);
        }
        return curBlock;
    },
    insertEmptyLine: function (blockContainer, after) {
        if (!blockContainer) {
            return;
        }
        var newLine = ENV.doc.createElement('div');
        var dom = ENV.doc.createElement('br');
        newLine.appendChild(dom);
        domUtils.before(newLine, blockContainer, after);
        tableZone.clear();
        rangeUtils.setRange(dom, 0);
    }
};

module.exports = blockUtils;
},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../editor/commandExtend":33,"../rangeUtils/rangeExtend":49,"../tableUtils/tableZone":55}],14:[function(require,module,exports){
/**
 * 代码区域操作核心包 core
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    dependLoader = require('../common/dependLoader'),
    historyUtils = require('../common/historyUtils'),
    blockUtils = require('../blockUtils/blockUtils'),
    domUtils = require('../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    codeUtils = require('./codeUtils'),
    codeStyle = require('./codeStyle');

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_SELECT_PLUGIN_CHANGE, _event.handler.onChangeSelector);
        ENV.event.add(CONST.EVENT.BEFORE_GET_DOCHTML, _event.handler.beforeGetDocHtml);
        ENV.event.add(CONST.EVENT.BEFORE_SAVESNAP, _event.handler.onBeforeSaveSnap);
        ENV.event.add(CONST.EVENT.AFTER_RESTORE_HISTORY, _event.handler.onAfterRestoreHistory);
        ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_SELECT_PLUGIN_CHANGE, _event.handler.onChangeSelector);
        ENV.event.remove(CONST.EVENT.BEFORE_GET_DOCHTML, _event.handler.beforeGetDocHtml);
        ENV.event.remove(CONST.EVENT.BEFORE_SAVESNAP, _event.handler.onBeforeSaveSnap);
        ENV.event.remove(CONST.EVENT.AFTER_RESTORE_HISTORY, _event.handler.onAfterRestoreHistory);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
    },
    handler: {
        beforeGetDocHtml: function () {
            codeUtils.saveToText();
            codeStyle.clearStyle();
        },
        onAfterRestoreHistory: function () {
            var containerList = codeUtils.getContainerList();
            var container, cm, cmDoc, i;
            for (i = containerList.length - 1; i >= 0; i--) {
                container = containerList[i];
                cm = container.codeMirror;
                if (!cm) {
                    // 恢复 版本（重新刷 body 后），实例化 CodeMirror
                    codeUtils.fixCodeContainer(container);
                    // 根据 history 恢复的 CodeMirror Doc 恢复当前 版本
                    cmDoc = historyUtils.getCodeMirrorDoc(container.id);
                    if (cmDoc) {
                        cm = container.codeMirror;
                        cm.swapDoc(cmDoc);
                    }
                }
            }
        },
        onBeforeSaveSnap: function () {
            var containerList = codeUtils.getContainerList();
            var container, cm, i;
            for (i = containerList.length - 1; i >= 0; i--) {
                container = containerList[i];
                cm = container.codeMirror;
                if (cm) {
                    // CodeMirror 将最新代码保存到 textarea
                    cm.save();
                    // 将 textarea 的 value 写入 html
                    domUtils.setTextarea(cm.getTextArea());
                }
            }
        },
        onChangeSelector: function (target) {
            var container;
            if (domUtils.hasClass(target, CONST.CLASS.CODE_TOOLS_MODE)) {
                container = codeUtils.getContainerFromChild(target);
                codeUtils.changeMode(container, target.value);
                container.codeMirror.focus();
                return;
            }
            if (domUtils.hasClass(target, CONST.CLASS.CODE_TOOLS_THEME)) {
                container = codeUtils.getContainerFromChild(target);
                codeStyle.insertTheme(target.value);
                codeUtils.changeTheme(container, target.value);
                container.codeMirror.focus();
                return;
            }
        },
        onMouseDown: function (e) {
            // 点击 Tools 的时候 阻止默认事件，避免与 CodeMirror 抢焦点
            var tools = codeUtils.getToolsFromChild(e.target);
            if (tools) {
                utils.stopEvent(e);
            }
        },
        onKeyDown: function (e) {
            if (codeUtils.onKeyDown(e)) {
                utils.stopEvent(e);
                return false;
            }
            return true;
        }
    }
};

var codeCore = {
    loadDependency: function (callback) {
        if (typeof CodeMirror === 'undefined') {
            dependLoader.loadJs(ENV.doc,
                dependLoader.getDependencyFiles(ENV.dependency, 'js', 'codeMirror'),
                function () {
                    callback();
                });
        } else {
            callback();
        }
    },
    on: function () {
        _event.bind();

        codeStyle.initCommon();
        // 修正旧的 prettyprint
        codeUtils.oldPatch.fixOldCode();
        // 不仅处理 旧的 code，还要初始化新的 codeMirror，所以不能使用 fixOldCode 的返回值
        codeUtils.fixCodeContainer();
    },
    off: function () {
        _event.unbind();
        codeUtils.clearCodeMirror();
    },
    insertCode: function () {
        historyUtils.saveSnap(false);
        var range = rangeUtils.getRange();

        if (!codeUtils.canCreateCode()) {
            // CodeMirror / Table 内
            return;
        }
        var curObj, container, textarea;
        var tempObj, tempRange, src = '';
        if (range) {
            if (!range.collapsed) {
                tempObj = ENV.doc.createElement('div');
                tempRange = range.cloneContents();
                while (tempRange.firstChild) {
                    tempObj.appendChild(tempRange.firstChild);
                }
                tempObj.style.position = "absolute";
                tempObj.style.top = "0px";
                tempObj.style.left = "-10000px";
                tempObj.style.height = "1px";
                ENV.doc.body.appendChild(tempObj);
                src = tempObj.innerText;
                ENV.doc.body.removeChild(tempObj);

                range.deleteContents();
            }

            curObj = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            curObj = domUtils.getBlockParent(curObj.container, true);
            if (curObj && curObj !== ENV.doc.body &&
                domUtils.isEmptyDom(curObj) && domUtils.isTag(curObj, 'div')) {
                container = curObj;
                curObj.innerHTML = '';
            }
        }

        textarea = ENV.doc.createElement('textarea');
        domUtils.setTextarea(textarea, src);
        container = container ? container : ENV.doc.createElement('div');
        container.appendChild(textarea);
        domUtils.addClass(container, CONST.CLASS.CODE_CONTAINER);
        // 只有新的 CodeMirror 才使用 last 设置
        container.isNew = true;
        if (!container.parentNode) {
            blockUtils.insertBlock(container);
        }
        codeUtils.fixCodeContainer(container);
        container.isNew = false;

        //修正 光标
        container.codeMirror.focus();

        ENV.event.call(CONST.EVENT.UPDATE_RENDER);
    },
    onKeyDown: _event.handler.onKeyDown
};

module.exports = codeCore;
},{"../blockUtils/blockUtils":13,"../common/const":18,"../common/dependLoader":19,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"./codeStyle":15,"./codeUtils":16}],15:[function(require,module,exports){
/**
 * codeMirror 相关样式处理
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    wizStyle = require('../common/wizStyle'),
    dependLoader = require('../common/dependLoader'),
    domUtils = require('../domUtils/domBase');

var CSS = {
    tmp: {
        common: '.wiz-code-tools {display:none;position: absolute; top: -32px; right: 0; opacity: .95; z-index: 10;}' +
        '.CodeMirror-focused .wiz-code-tools {display:block;}' +
        '.CodeMirror-sizer {border-right: 0 !important;}' +
        'body pre.prettyprint {padding:0;}' +
        'body pre.prettyprint code {white-space: pre;}' +
        'body pre.prettyprint.linenums {box-shadow:none; overflow: auto;-webkit-overflow-scrolling: touch;}' +
        'body pre.prettyprint.linenums ol.linenums {box-shadow: 40px 0 0 #FBFBFC inset, 41px 0 0 #ECECF0 inset; padding: 10px 10px 10px 40px !important;}',
        reader: '.CodeMirror-cursors {visibility: hidden !important;}',
        phone: '.' + CONST.CLASS.CODE_CONTAINER + '{margin-left:0; margin-right:0;}'
    },
    common: '.' + CONST.CLASS.CODE_CONTAINER + '{position: relative; padding:8px 0; margin: 5px 25px 5px 5px;text-indent:0; text-align:left;}' + // 专门用于 CodeMirror 之间间隔，并且可以自动添加空行
    '.CodeMirror {font-family: Consolas, "Liberation Mono", Menlo, Courier, monospace; color: black; font-size: 10pt; font-size: 0.83rem}' +
    '.CodeMirror-lines {padding: 4px 0;}' +
    '.CodeMirror pre {padding: 0 4px;}' +
    '.CodeMirror-scrollbar-filler, .CodeMirror-gutter-filler {background-color: white;}' +
    '.CodeMirror-gutters {border-right: 1px solid #ddd; background-color: #f7f7f7; white-space: nowrap;}' +
    '.CodeMirror-linenumbers {}' +
    '.CodeMirror-linenumber {padding: 0 3px 0 5px; min-width: 20px; text-align: right; color: #999; white-space: nowrap;}' +
    '.CodeMirror-guttermarker {color: black;}' +
    '.CodeMirror-guttermarker-subtle {color: #999;}' +
    '.CodeMirror-cursor {border-left: 1px solid black; border-right: none; width: 0;}' +
    '.CodeMirror div.CodeMirror-secondarycursor {border-left: 1px solid silver;}' +
    '.cm-fat-cursor .CodeMirror-cursor {width: auto; border: 0 !important; background: #7e7;}' +
    '.cm-fat-cursor div.CodeMirror-cursors {z-index: 1;}' +
    '.cm-animate-fat-cursor {width: auto; border: 0; -webkit-animation: blink 1.06s steps(1) infinite; -moz-animation: blink 1.06s steps(1) infinite; animation: blink 1.06s steps(1) infinite; background-color: #7e7;}' +
    '@-moz-keyframes blink {' +
    '  0% {}' +
    '  50% { background-color: transparent; }' +
    '  100% {}' +
    '}' +
    '@-webkit-keyframes blink {' +
    '  0% {}' +
    '  50% { background-color: transparent; }' +
    '  100% {}' +
    '}' +
    '@keyframes blink {' +
    '  0% {}' +
    '  50% { background-color: transparent; }' +
    '  100% {}' +
    '}' +
    '.CodeMirror-overwrite .CodeMirror-cursor {}' +
    '.cm-tab { display: inline-block; text-decoration: inherit; }' +
    '.CodeMirror-rulers {position: absolute; left: 0; right: 0; top: -50px; bottom: -20px; overflow: hidden;}' +
    '.CodeMirror-ruler {border-left: 1px solid #ccc; top: 0; bottom: 0; position: absolute;}' +
    '.cm-s-default .cm-header {color: blue;}' +
    '.cm-s-default .cm-quote {color: #090;}' +
    '.cm-negative {color: #d44;}' +
    '.cm-positive {color: #292;}' +
    '.cm-header, .cm-strong {font-weight: bold;}' +
    '.cm-em {font-style: italic;}' +
    '.cm-link {text-decoration: underline;}' +
    '.cm-strikethrough {text-decoration: line-through;}' +
    '.cm-s-default .cm-keyword {color: #708;}' +
    '.cm-s-default .cm-atom {color: #219;}' +
    '.cm-s-default .cm-number {color: #164;}' +
    '.cm-s-default .cm-def {color: #00f;}' +
    '.cm-s-default .cm-variable,' +
    '.cm-s-default .cm-punctuation,' +
    '.cm-s-default .cm-property,' +
    '.cm-s-default .cm-operator {}' +
    '.cm-s-default .cm-variable-2 {color: #05a;}' +
    '.cm-s-default .cm-variable-3 {color: #085;}' +
    '.cm-s-default .cm-comment {color: #a50;}' +
    '.cm-s-default .cm-string {color: #a11;}' +
    '.cm-s-default .cm-string-2 {color: #f50;}' +
    '.cm-s-default .cm-meta {color: #555;}' +
    '.cm-s-default .cm-qualifier {color: #555;}' +
    '.cm-s-default .cm-builtin {color: #30a;}' +
    '.cm-s-default .cm-bracket {color: #997;}' +
    '.cm-s-default .cm-tag {color: #170;}' +
    '.cm-s-default .cm-attribute {color: #00c;}' +
    '.cm-s-default .cm-hr {color: #999;}' +
    '.cm-s-default .cm-link {color: #00c;}' +
    '.cm-s-default .cm-error {color: #f00;}' +
    '.cm-invalidchar {color: #f00;}' +
    '.CodeMirror-composing { border-bottom: 2px solid; }' +
    'div.CodeMirror span.CodeMirror-matchingbracket {color: #0f0;}' +
    'div.CodeMirror span.CodeMirror-nonmatchingbracket {color: #f22;}' +
    '.CodeMirror-matchingtag { background: rgba(255, 150, 0, .3); }' +
    '.CodeMirror-activeline-background {background: #e8f2ff;}' +
    // '.CodeMirror {position: relative; overflow: hidden; background: white;}' +
    '.CodeMirror {position: relative; background: #f5f5f5;}' +
    // '.CodeMirror-scroll {overflow: scroll !important; margin-bottom: -30px; margin-right: -30px; padding-bottom: 30px; height: 100%; outline: none; position: relative;}' +
    '.CodeMirror-scroll {overflow: hidden !important; margin-bottom: 0; margin-right: -30px; padding: 16px 30px 16px 0; outline: none; position: relative;}' +
    '.CodeMirror-sizer {position: relative; border-right: 30px solid transparent;}' +
    '.CodeMirror-vscrollbar, .CodeMirror-hscrollbar, .CodeMirror-scrollbar-filler, .CodeMirror-gutter-filler {position: absolute; z-index: 6; display: none;}' +
    '.CodeMirror-vscrollbar {right: 0; top: 0; overflow-x: hidden; overflow-y: scroll;}' +
    '.CodeMirror-hscrollbar {bottom: 0; left: 0 !important; overflow-y: hidden; overflow-x: scroll;}' +
    '.CodeMirror-scrollbar-filler {right: 0; bottom: 0;}' +
    '.CodeMirror-gutter-filler {left: 0; bottom: 0;}' +
    // '.CodeMirror-gutters {position: absolute; left: 0; top: 0; min-height: 100%; z-index: 3;}' +
    '.CodeMirror-gutters {position: absolute; left: 0; top: -5px; min-height: 100%; z-index: 3;}' +
    '.CodeMirror-gutter {white-space: normal; height: inherit; display: inline-block; vertical-align: top; margin-bottom: -30px;}' +
    '.CodeMirror-gutter-wrapper {position: absolute; z-index: 4; background: none !important; border: none !important;}' +
    '.CodeMirror-gutter-background {position: absolute; top: 0; bottom: 0; z-index: 4;}' +
    '.CodeMirror-gutter-elt {position: absolute; cursor: default; z-index: 4; text-align: center;}' +
    '.CodeMirror-gutter-wrapper ::selection { background-color: transparent }' +
    '.CodeMirror-gutter-wrapper ::-moz-selection { background-color: transparent }' +
    '.CodeMirror-lines {cursor: text; min-height: 1px;}' +
    '.CodeMirror pre {-moz-border-radius: 0; -webkit-border-radius: 0; border-radius: 0; border-width: 0; background: transparent; font-family: inherit; font-size: inherit; margin: 0; white-space: pre; word-wrap: normal; line-height: inherit; color: inherit; z-index: 2; position: relative; overflow: visible; -webkit-tap-highlight-color: transparent; -webkit-font-variant-ligatures: contextual; font-variant-ligatures: contextual;}' +
    '.CodeMirror-wrap pre {word-wrap: break-word; white-space: pre-wrap; word-break: normal;}' +
    '.CodeMirror-linebackground {position: absolute; left: 0; right: 0; top: 0; bottom: 0; z-index: 0;}' +
    '.CodeMirror-linewidget {position: relative; z-index: 2; overflow: auto;}' +
    '.CodeMirror-widget {}' +
    '.CodeMirror-rtl pre { direction: rtl; }' +
    '.CodeMirror-code {outline: none;}' +
    '.CodeMirror-scroll,.CodeMirror-sizer,.CodeMirror-gutter,.CodeMirror-gutters,.CodeMirror-linenumber {-moz-box-sizing: content-box; box-sizing: content-box;}' +
    '.CodeMirror-measure {position: absolute; width: 100%; height: 0; overflow: hidden; visibility: hidden;}' +
    '.CodeMirror-cursor {position: absolute; pointer-events: none;}' +
    '.CodeMirror-measure pre { position: static; }' +
    'div.CodeMirror-cursors {visibility: hidden; position: relative; z-index: 3;}' +
    'div.CodeMirror-dragcursors {visibility: visible;}' +
    '.CodeMirror-focused div.CodeMirror-cursors {visibility: visible;}' +
    '.CodeMirror-selected { background: #d9d9d9; }' +
    '.CodeMirror-focused .CodeMirror-selected { background: #d7d4f0; }' +
    '.CodeMirror-crosshair { cursor: crosshair; }' +
    '.CodeMirror-line::selection, .CodeMirror-line > span::selection, .CodeMirror-line > span > span::selection { background: #d7d4f0; }' +
    '.CodeMirror-line::-moz-selection, .CodeMirror-line > span::-moz-selection, .CodeMirror-line > span > span::-moz-selection { background: #d7d4f0; }' +
    '.cm-searching {background: #ffa; background: rgba(255, 255, 0, .4);}' +
    '.cm-force-border { padding-right: .1px; }' +
    '@media print {' +
    '  .CodeMirror div.CodeMirror-cursors {visibility: hidden;}' +
    '}' +
    '.cm-tab-wrap-hack:after { content: ""; }' +
    'span.CodeMirror-selectedtext { background: none; }' +
    '.CodeMirror-activeline-background, .CodeMirror-selected {transition: visibility 0ms 100ms;}' +
    '.CodeMirror-blur .CodeMirror-activeline-background, .CodeMirror-blur .CodeMirror-selected {visibility:hidden;}' +
    '.CodeMirror-blur .CodeMirror-matchingbracket {color:inherit !important;outline:none !important;text-decoration:none !important;}' +
    '',
    theme: {
        'base16-dark': '.cm-s-base16-dark.CodeMirror { background: #151515; color: #e0e0e0; }' +
        '.cm-s-base16-dark div.CodeMirror-selected { background: #303030; }' +
        '.cm-s-base16-dark .CodeMirror-line::selection, .cm-s-base16-dark .CodeMirror-line > span::selection, .cm-s-base16-dark .CodeMirror-line > span > span::selection { background: rgba(48, 48, 48, .99); }' +
        '.cm-s-base16-dark .CodeMirror-line::-moz-selection, .cm-s-base16-dark .CodeMirror-line > span::-moz-selection, .cm-s-base16-dark .CodeMirror-line > span > span::-moz-selection { background: rgba(48, 48, 48, .99); }' +
        '.cm-s-base16-dark .CodeMirror-gutters { background: #151515; border-right: 0px; }' +
        '.cm-s-base16-dark .CodeMirror-guttermarker { color: #ac4142; }' +
        '.cm-s-base16-dark .CodeMirror-guttermarker-subtle { color: #505050; }' +
        '.cm-s-base16-dark .CodeMirror-linenumber { color: #505050; }' +
        '.cm-s-base16-dark .CodeMirror-cursor { border-left: 1px solid #b0b0b0; }' +
        '.cm-s-base16-dark span.cm-comment { color: #8f5536; }' +
        '.cm-s-base16-dark span.cm-atom { color: #aa759f; }' +
        '.cm-s-base16-dark span.cm-number { color: #aa759f; }' +
        '.cm-s-base16-dark span.cm-property, .cm-s-base16-dark span.cm-attribute { color: #90a959; }' +
        '.cm-s-base16-dark span.cm-keyword { color: #ac4142; }' +
        '.cm-s-base16-dark span.cm-string { color: #f4bf75; }' +
        '.cm-s-base16-dark span.cm-variable { color: #90a959; }' +
        '.cm-s-base16-dark span.cm-variable-2 { color: #6a9fb5; }' +
        '.cm-s-base16-dark span.cm-def { color: #d28445; }' +
        '.cm-s-base16-dark span.cm-bracket { color: #e0e0e0; }' +
        '.cm-s-base16-dark span.cm-tag { color: #ac4142; }' +
        '.cm-s-base16-dark span.cm-link { color: #aa759f; }' +
        '.cm-s-base16-dark span.cm-error { background: #ac4142; color: #b0b0b0; }' +
        '.cm-s-base16-dark .CodeMirror-activeline-background { background: #202020; }' +
        '.cm-s-base16-dark .CodeMirror-matchingbracket { text-decoration: underline; color: white !important; }',

        'base16-light': '.cm-s-base16-light.CodeMirror { background: #f5f5f5; color: #202020; }' +
        '.cm-s-base16-light div.CodeMirror-selected { background: #e0e0e0; }' +
        '.cm-s-base16-light .CodeMirror-line::selection, .cm-s-base16-light .CodeMirror-line > span::selection, .cm-s-base16-light .CodeMirror-line > span > span::selection { background: #e0e0e0; }' +
        '.cm-s-base16-light .CodeMirror-line::-moz-selection, .cm-s-base16-light .CodeMirror-line > span::-moz-selection, .cm-s-base16-light .CodeMirror-line > span > span::-moz-selection { background: #e0e0e0; }' +
        '.cm-s-base16-light .CodeMirror-gutters { background: #f5f5f5; border-right: 0px; }' +
        '.cm-s-base16-light .CodeMirror-guttermarker { color: #ac4142; }' +
        '.cm-s-base16-light .CodeMirror-guttermarker-subtle { color: #b0b0b0; }' +
        '.cm-s-base16-light .CodeMirror-linenumber { color: #b0b0b0; }' +
        '.cm-s-base16-light .CodeMirror-cursor { border-left: 1px solid #505050; }' +
        '.cm-s-base16-light span.cm-comment { color: #8f5536; }' +
        '.cm-s-base16-light span.cm-atom { color: #aa759f; }' +
        '.cm-s-base16-light span.cm-number { color: #aa759f; }' +
        '.cm-s-base16-light span.cm-property, .cm-s-base16-light span.cm-attribute { color: #90a959; }' +
        '.cm-s-base16-light span.cm-keyword { color: #ac4142; }' +
        '.cm-s-base16-light span.cm-string { color: #f4bf75; }' +
        '.cm-s-base16-light span.cm-variable { color: #90a959; }' +
        '.cm-s-base16-light span.cm-variable-2 { color: #6a9fb5; }' +
        '.cm-s-base16-light span.cm-def { color: #d28445; }' +
        '.cm-s-base16-light span.cm-bracket { color: #202020; }' +
        '.cm-s-base16-light span.cm-tag { color: #ac4142; }' +
        '.cm-s-base16-light span.cm-link { color: #aa759f; }' +
        '.cm-s-base16-light span.cm-error { background: #ac4142; color: #505050; }' +
        '.cm-s-base16-light .CodeMirror-activeline-background { background: #DDDCDC; }' +
        '.cm-s-base16-light .CodeMirror-matchingbracket { text-decoration: underline; color: white !important; }',

        'blackboard': '.cm-s-blackboard.CodeMirror { background: #0C1021; color: #F8F8F8; }' +
        '.cm-s-blackboard div.CodeMirror-selected { background: #253B76; }' +
        '.cm-s-blackboard .CodeMirror-line::selection, .cm-s-blackboard .CodeMirror-line > span::selection, .cm-s-blackboard .CodeMirror-line > span > span::selection { background: rgba(37, 59, 118, .99); }' +
        '.cm-s-blackboard .CodeMirror-line::-moz-selection, .cm-s-blackboard .CodeMirror-line > span::-moz-selection, .cm-s-blackboard .CodeMirror-line > span > span::-moz-selection { background: rgba(37, 59, 118, .99); }' +
        '.cm-s-blackboard .CodeMirror-gutters { background: #0C1021; border-right: 0; }' +
        '.cm-s-blackboard .CodeMirror-guttermarker { color: #FBDE2D; }' +
        '.cm-s-blackboard .CodeMirror-guttermarker-subtle { color: #888; }' +
        '.cm-s-blackboard .CodeMirror-linenumber { color: #888; }' +
        '.cm-s-blackboard .CodeMirror-cursor { border-left: 1px solid #A7A7A7; }' +
        '.cm-s-blackboard .cm-keyword { color: #FBDE2D; }' +
        '.cm-s-blackboard .cm-atom { color: #D8FA3C; }' +
        '.cm-s-blackboard .cm-number { color: #D8FA3C; }' +
        '.cm-s-blackboard .cm-def { color: #8DA6CE; }' +
        '.cm-s-blackboard .cm-variable { color: #FF6400; }' +
        '.cm-s-blackboard .cm-operator { color: #FBDE2D; }' +
        '.cm-s-blackboard .cm-comment { color: #AEAEAE; }' +
        '.cm-s-blackboard .cm-string { color: #61CE3C; }' +
        '.cm-s-blackboard .cm-string-2 { color: #61CE3C; }' +
        '.cm-s-blackboard .cm-meta { color: #D8FA3C; }' +
        '.cm-s-blackboard .cm-builtin { color: #8DA6CE; }' +
        '.cm-s-blackboard .cm-tag { color: #8DA6CE; }' +
        '.cm-s-blackboard .cm-attribute { color: #8DA6CE; }' +
        '.cm-s-blackboard .cm-header { color: #FF6400; }' +
        '.cm-s-blackboard .cm-hr { color: #AEAEAE; }' +
        '.cm-s-blackboard .cm-link { color: #8DA6CE; }' +
        '.cm-s-blackboard .cm-error { background: #9D1E15; color: #F8F8F8; }' +
        '.cm-s-blackboard .CodeMirror-activeline-background { background: #3C3636; }' +
        '.cm-s-blackboard .CodeMirror-matchingbracket { outline:1px solid grey;color:white !important; }',

        'eclipse': '.cm-s-eclipse span.cm-meta { color: #FF1717; }' +
        '.cm-s-eclipse span.cm-keyword { line-height: 1em; font-weight: bold; color: #7F0055; }' +
        '.cm-s-eclipse span.cm-atom { color: #219; }' +
        '.cm-s-eclipse span.cm-number { color: #164; }' +
        '.cm-s-eclipse span.cm-def { color: #00f; }' +
        '.cm-s-eclipse span.cm-variable { color: black; }' +
        '.cm-s-eclipse span.cm-variable-2 { color: #0000C0; }' +
        '.cm-s-eclipse span.cm-variable-3 { color: #0000C0; }' +
        '.cm-s-eclipse span.cm-property { color: black; }' +
        '.cm-s-eclipse span.cm-operator { color: black; }' +
        '.cm-s-eclipse span.cm-comment { color: #3F7F5F; }' +
        '.cm-s-eclipse span.cm-string { color: #2A00FF; }' +
        '.cm-s-eclipse span.cm-string-2 { color: #f50; }' +
        '.cm-s-eclipse span.cm-qualifier { color: #555; }' +
        '.cm-s-eclipse span.cm-builtin { color: #30a; }' +
        '.cm-s-eclipse span.cm-bracket { color: #cc7; }' +
        '.cm-s-eclipse span.cm-tag { color: #170; }' +
        '.cm-s-eclipse span.cm-attribute { color: #00c; }' +
        '.cm-s-eclipse span.cm-link { color: #219; }' +
        '.cm-s-eclipse span.cm-error { color: #f00; }' +
        '.cm-s-eclipse .CodeMirror-activeline-background { background: #e8f2ff; }' +
        '.cm-s-eclipse .CodeMirror-matchingbracket { outline:1px solid grey; color:black !important; }',

        'material': '.cm-s-material.CodeMirror {background-color: #263238; color: rgba(233, 237, 237, 1);}' +
        '.cm-s-material .CodeMirror-gutters {background: #263238; color: rgb(83,127,126); border: none;}' +
        '.cm-s-material .CodeMirror-guttermarker, .cm-s-material .CodeMirror-guttermarker-subtle, .cm-s-material .CodeMirror-linenumber { color: rgb(83,127,126); }' +
        '.cm-s-material .CodeMirror-cursor { border-left: 1px solid #f8f8f0; }' +
        '.cm-s-material div.CodeMirror-selected { background: rgba(255, 255, 255, 0.15); }' +
        '.cm-s-material.CodeMirror-focused div.CodeMirror-selected { background: rgba(255, 255, 255, 0.10); }' +
        '.cm-s-material .CodeMirror-line::selection, .cm-s-material .CodeMirror-line > span::selection, .cm-s-material .CodeMirror-line > span > span::selection { background: rgba(255, 255, 255, 0.10); }' +
        '.cm-s-material .CodeMirror-line::-moz-selection, .cm-s-material .CodeMirror-line > span::-moz-selection, .cm-s-material .CodeMirror-line > span > span::-moz-selection { background: rgba(255, 255, 255, 0.10); }' +
        '.cm-s-material .CodeMirror-activeline-background { background: rgba(0, 0, 0, 0); }' +
        '.cm-s-material .cm-keyword { color: rgba(199, 146, 234, 1); }' +
        '.cm-s-material .cm-operator { color: rgba(233, 237, 237, 1); }' +
        '.cm-s-material .cm-variable-2 { color: #80CBC4; }' +
        '.cm-s-material .cm-variable-3 { color: #82B1FF; }' +
        '.cm-s-material .cm-builtin { color: #DECB6B; }' +
        '.cm-s-material .cm-atom { color: #F77669; }' +
        '.cm-s-material .cm-number { color: #F77669; }' +
        '.cm-s-material .cm-def { color: rgba(233, 237, 237, 1); }' +
        '.cm-s-material .cm-string { color: #C3E88D; }' +
        '.cm-s-material .cm-string-2 { color: #80CBC4; }' +
        '.cm-s-material .cm-comment { color: #546E7A; }' +
        '.cm-s-material .cm-variable { color: #82B1FF; }' +
        '.cm-s-material .cm-tag { color: #80CBC4; }' +
        '.cm-s-material .cm-meta { color: #80CBC4; }' +
        '.cm-s-material .cm-attribute { color: #FFCB6B; }' +
        '.cm-s-material .cm-property { color: #80CBAE; }' +
        '.cm-s-material .cm-qualifier { color: #DECB6B; }' +
        '.cm-s-material .cm-variable-3 { color: #DECB6B; }' +
        '.cm-s-material .cm-tag { color: rgba(255, 83, 112, 1); }' +
        '.cm-s-material .cm-error {color: rgba(255, 255, 255, 1.0); background-color: #EC5F67;}' +
        '.cm-s-material .CodeMirror-matchingbracket {text-decoration: underline; color: white !important;}',

        'monokai': '.cm-s-monokai.CodeMirror { background: #272822; color: #f8f8f2; }' +
        '.cm-s-monokai div.CodeMirror-selected { background: #49483E; }' +
        '.cm-s-monokai .CodeMirror-line::selection, .cm-s-monokai .CodeMirror-line > span::selection, .cm-s-monokai .CodeMirror-line > span > span::selection { background: rgba(73, 72, 62, .99); }' +
        '.cm-s-monokai .CodeMirror-line::-moz-selection, .cm-s-monokai .CodeMirror-line > span::-moz-selection, .cm-s-monokai .CodeMirror-line > span > span::-moz-selection { background: rgba(73, 72, 62, .99); }' +
        '.cm-s-monokai .CodeMirror-gutters { background: #272822; border-right: 0px; }' +
        '.cm-s-monokai .CodeMirror-guttermarker { color: white; }' +
        '.cm-s-monokai .CodeMirror-guttermarker-subtle { color: #d0d0d0; }' +
        '.cm-s-monokai .CodeMirror-linenumber { color: #d0d0d0; }' +
        '.cm-s-monokai .CodeMirror-cursor { border-left: 1px solid #f8f8f0; }' +
        '.cm-s-monokai span.cm-comment { color: #75715e; }' +
        '.cm-s-monokai span.cm-atom { color: #ae81ff; }' +
        '.cm-s-monokai span.cm-number { color: #ae81ff; }' +
        '.cm-s-monokai span.cm-property, .cm-s-monokai span.cm-attribute { color: #a6e22e; }' +
        '.cm-s-monokai span.cm-keyword { color: #f92672; }' +
        '.cm-s-monokai span.cm-builtin { color: #66d9ef; }' +
        '.cm-s-monokai span.cm-string { color: #e6db74; }' +
        '.cm-s-monokai span.cm-variable { color: #f8f8f2; }' +
        '.cm-s-monokai span.cm-variable-2 { color: #9effff; }' +
        '.cm-s-monokai span.cm-variable-3 { color: #66d9ef; }' +
        '.cm-s-monokai span.cm-def { color: #fd971f; }' +
        '.cm-s-monokai span.cm-bracket { color: #f8f8f2; }' +
        '.cm-s-monokai span.cm-tag { color: #f92672; }' +
        '.cm-s-monokai span.cm-header { color: #ae81ff; }' +
        '.cm-s-monokai span.cm-link { color: #ae81ff; }' +
        '.cm-s-monokai span.cm-error { background: #f92672; color: #f8f8f0; }' +
        '.cm-s-monokai .CodeMirror-activeline-background { background: #373831; }' +
        '.cm-s-monokai .CodeMirror-matchingbracket {text-decoration: underline; color: white !important;}',

        'tomorrow-night-eighties': '.cm-s-tomorrow-night-eighties.CodeMirror { background: #000000; color: #CCCCCC; }' +
        '.cm-s-tomorrow-night-eighties div.CodeMirror-selected { background: #2D2D2D; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-line::selection, .cm-s-tomorrow-night-eighties .CodeMirror-line > span::selection, .cm-s-tomorrow-night-eighties .CodeMirror-line > span > span::selection { background: rgba(45, 45, 45, 0.99); }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-line::-moz-selection, .cm-s-tomorrow-night-eighties .CodeMirror-line > span::-moz-selection, .cm-s-tomorrow-night-eighties .CodeMirror-line > span > span::-moz-selection { background: rgba(45, 45, 45, 0.99); }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-gutters { background: #000000; border-right: 0px; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-guttermarker { color: #f2777a; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-guttermarker-subtle { color: #777; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-linenumber { color: #515151; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-cursor { border-left: 1px solid #6A6A6A; }' +
        '.cm-s-tomorrow-night-eighties span.cm-comment { color: #d27b53; }' +
        '.cm-s-tomorrow-night-eighties span.cm-atom { color: #a16a94; }' +
        '.cm-s-tomorrow-night-eighties span.cm-number { color: #a16a94; }' +
        '.cm-s-tomorrow-night-eighties span.cm-property, .cm-s-tomorrow-night-eighties span.cm-attribute { color: #99cc99; }' +
        '.cm-s-tomorrow-night-eighties span.cm-keyword { color: #f2777a; }' +
        '.cm-s-tomorrow-night-eighties span.cm-string { color: #ffcc66; }' +
        '.cm-s-tomorrow-night-eighties span.cm-variable { color: #99cc99; }' +
        '.cm-s-tomorrow-night-eighties span.cm-variable-2 { color: #6699cc; }' +
        '.cm-s-tomorrow-night-eighties span.cm-def { color: #f99157; }' +
        '.cm-s-tomorrow-night-eighties span.cm-bracket { color: #CCCCCC; }' +
        '.cm-s-tomorrow-night-eighties span.cm-tag { color: #f2777a; }' +
        '.cm-s-tomorrow-night-eighties span.cm-link { color: #a16a94; }' +
        '.cm-s-tomorrow-night-eighties span.cm-error { background: #f2777a; color: #6A6A6A; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-activeline-background { background: #343600; }' +
        '.cm-s-tomorrow-night-eighties .CodeMirror-matchingbracket { text-decoration: underline; color: white !important; }'
    }
};

//

var codeStyle = {
    clearStyle: function () {
        var styleList = ENV.doc.getElementsByName(CONST.NAME.CODE_STYLE);
        var style, theme, i;
        for (i = styleList.length - 1; i >= 0; i--) {
            style = styleList[i];
            theme = style.id.replace(CONST.ID.CODE_STYLE + '-', '');
            if (!ENV.doc.querySelector('.' + CONST.CLASS.CODE_CONTAINER + '[data-theme=' + theme + ']')) {
                wizStyle.removeStyleById(style.id);
            }
        }
        var codeList = ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER);
        if (codeList.length === 0) {
            wizStyle.removeStyleById(CONST.ID.CODE_STYLE);
        }
    },
    initCommon: function() {
        wizStyle.replaceStyleById(CONST.ID.CODE_STYLE, CSS.common, false);
        codeStyle.insertTemp();
    },
    insertCommon: function() {
        var codeStyle = ENV.doc.getElementById(CONST.ID.CODE_STYLE);
        if (!codeStyle) {
            wizStyle.replaceStyleById(CONST.ID.CODE_STYLE, CSS.common, false);
        }
    },
    insertTemp: function () {
        wizStyle.insertStyle({
            name: CONST.NAME.TMP_STYLE
        }, CSS.tmp.common);

        if (ENV.client.type.isPhone || ENV.client.type.isPad) {
            wizStyle.insertStyle({
                name: CONST.NAME.TMP_STYLE
            }, CSS.tmp.phone);
        }

        if (ENV.readonly) {
            wizStyle.insertStyle({
                name: CONST.NAME.TMP_STYLE
            }, CSS.tmp.reader);
        }
    },
    insertTheme: function (theme) {
        if (CSS.theme[theme] && !ENV.doc.querySelector('#' + CONST.ID.CODE_STYLE + '-' + theme)) {
            wizStyle.insertStyle({
                id: CONST.ID.CODE_STYLE + '-' + theme,
                name: CONST.NAME.CODE_STYLE
            }, CSS.theme[theme]);
        }
    }
};

module.exports = codeStyle;

},{"../common/const":18,"../common/dependLoader":19,"../common/env":20,"../common/wizStyle":25,"../domUtils/domBase":28}],16:[function(require,module,exports){
/**
 * 代码区域操作的基本方法集合
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    domUtils = require('../domUtils/domExtend'),
    selectPlugin = require('../domUtils/selectPlugin'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    codeStyle = require('./codeStyle');

var CODE_MIRROR = {
    MODE: {
        'APL': {
            cm: 'text/apl',
            file: 'apl',
            same: ['apollo']
        },
        'C': {
            cm: 'text/x-csrc',
            file: 'clike',
            same: []
        },
        'C++': {
            cm: 'text/x-c++src',
            file: 'clike',
            same: ['cpp']
        },
        'C#': {
            cm: 'text/x-csharp',
            file: 'clike',
            same: ['cs']
        },
        'CSS': {
            cm: 'text/css',
            file: 'css',
            same: []
        },
        'Erlang': {
            cm: 'text/x-erlang',
            file: 'erlang',
            same: []
        },
        'Go': {
            cm: 'text/x-go',
            file: 'go',
            same: []
        },
        'HTML': {
            cm: 'text/html',
            file: [
                ['xml', 'css', 'javascript', 'vbscript'],
                ['htmlmixed']
            ],
            same: []
        },
        'Java': {
            cm: 'text/x-java',
            file: 'clike',
            same: []
        },
        'JavaScript': {
            cm: 'text/javascript',
            file: 'javascript',
            same: ['js']
        },
        'JSX': {
            cm: 'text/jsx',
            file: [
                ['xml', 'javascript'],
                ['jsx']
            ],
            same: []
        },
        'Lua': {
            cm: 'text/x-lua',
            file: 'lua',
            same: []
        },
        'Octave (MATLAB)': {
            cm: 'text/x-octave',
            file: 'octave',
            same: ['matlab', 'octave']
        },
        'Objective-C': {
            cm: 'text/x-objectivec',
            file: 'clike',
            same: []
        },
        'Pascal': {
            cm: 'text/x-pascal',
            file: 'pascal',
            same: []
        },
        'Perl': {
            cm: 'text/x-perl',
            file: 'perl',
            same: []
        },
        'PHP': {
            cm: 'application/x-httpd-php',
            file: 'php',
            same: []
        },
        'Python': {
            cm: 'text/x-python',
            file: 'python',
            same: ['py']
        },
        'Ruby': {
            cm: 'text/x-ruby',
            file: 'ruby',
            same: ['rb']
        },
        'Shell': {
            cm: 'text/x-sh',
            file: 'shell',
            same: ['sh']
        },
        'SQL': {
            cm: 'text/x-sql',
            file: 'sql',
            same: []
        },
        'Swift': {
            cm: 'text/x-swift',
            file: 'swift',
            same: []
        },
        'VBScript': {
            cm: 'text/vbscript',
            file: 'vbscript',
            same: ['basic', 'vb']
        },
        'Verilog': {
            cm: 'text/x-verilog',
            file: 'verilog',
            same: []
        },
        'VHDL': {
            cm: 'text/x-vhdl',
            file: 'vhdl',
            same: []
        },
        'XML': {
            cm: 'application/xml',
            file: 'xml',
            same: []
        },
        'XSL': {
            cm: 'application/xml',
            file: 'xml',
            same: []
        },
        'YAML': {
            cm: 'text/x-yaml',
            file: 'yaml',
            same: []
        }
    },
    THEME: {
        'default': {
            name: 'L1 (default)',
            same: ['L1']
        },
        'base16-light': {
            name: 'L2 (base16-light)',
            same: ['L2']
        },
        'eclipse': {
            name: 'L3 (eclipse)',
            same: ['L3']
        },
        'base16-dark': {
            name: 'D1 (base16-dark)',
            same: ['D1']
        },
        'blackboard': {
            name: 'D2 (blackboard)',
            same: ['D2']
        },
        'material': {
            name: 'D3 (material)',
            same: ['D3']
        },
        'monokai': {
            name: 'D4 (monokai)',
            same: ['D4']
        },
        'tomorrow-night-eighties': {
            name: 'D5 (tomorrow)',
            same: ['D5']
        }
    }
};

var ModeOptions = [];
var ThemeOptions = [];
var modeDic = {};
var themeDic = {};

(function () {
    var key, same, i;
    for (key in CODE_MIRROR.MODE) {
        if (CODE_MIRROR.MODE.hasOwnProperty(key)) {
            ModeOptions.push({
                text: key,
                value: key
            });
            same = CODE_MIRROR.MODE[key].same;
            modeDic[key.toLowerCase()] = key;
            for (i = 0; i < same.length; i++) {
                modeDic[same[i].toLowerCase()] = key;
            }
        }
    }
    for (key in CODE_MIRROR.THEME) {
        if (CODE_MIRROR.THEME.hasOwnProperty(key)) {
            ThemeOptions.push({
                text: CODE_MIRROR.THEME[key].name,
                value: key
            });
            same = CODE_MIRROR.THEME[key].same;
            themeDic[key.toLowerCase()] = key;
            for (i = 0; i < same.length; i++) {
                themeDic[same[i].toLowerCase()] = key;
            }
        }
    }
})();


var codeKey = {
    mode: 'wiz-code-mode',
    theme: 'wiz-code-theme'
};

var codeUtils = {
    canCreateCode: function () {
        var range = rangeUtils.getRange();
        if (!range ||
            codeUtils.getContainerFromChild(range.startContainer) ||
            domUtils.getParentByClass(range.startContainer, CONST.CLASS.TABLE_CONTAINER, true)) {
            // CodeMirror / Table 内
            return false;
        }
        return true;
    },
    changeMode: function (container, mode) {
        if (!container) {
            return;
        }
        var cm = container.codeMirror;
        if (!cm) {
            return;
        }
        cm.setOption('mode', CODE_MIRROR.MODE[mode].cm);
        domUtils.attr(container, {
            'data-mode': mode
        });
        if (ENV.win.localStorage && !ENV.readonly) {
            ENV.win.localStorage.setItem(codeKey.mode, mode);
        }
    },
    changeTheme: function (container, theme) {
        if (!container) {
            return;
        }
        theme = theme || 'default';

        var cm = container.codeMirror;
        if (!cm) {
            return;
        }
        codeStyle.insertTheme(theme);

        cm.setOption('theme', theme);
        domUtils.attr(container, {
            'data-theme': theme
        });
        if (ENV.win.localStorage && !ENV.readonly) {
            ENV.win.localStorage.setItem(codeKey.theme, theme);
        }
    },
    clearCodeMirror: function () {
        var containerList = codeUtils.getContainerList();
        var container, cm, i;
        for (i = containerList.length - 1; i >= 0; i--) {
            container = containerList[i];
            if (container.codeMirror) {
                cm = container.codeMirror;
                cm.setOption('readOnly', true);
                cm.setOption('styleActiveLine', false);
                cm.getDoc().clearHistory();
            }
        }
    },
    fixCodeContainer: function (_container) {
        var containerList = _container ? [_container] : codeUtils.getContainerList();
        var container, i;
        for (i = containerList.length - 1; i >= 0; i--) {
            container = containerList[i];

            // codemirror 容器 的 style/id/class 必须清除
            container.removeAttribute('style');
            container.removeAttribute('id');
            container.removeAttribute('class');

            codeUtils.fixCodeContainerParent(container);
            if (ENV.options.codeNoIDE) {
                codeUtils.initCodeNoIDE(container);
            } else {
                codeUtils.initCodeMirror(container);
            }
        }
    },
    fixCodeContainerParent: function(container) {
        var parentList, parent;
        parentList = domUtils.getParentList(container);
        if (parentList.length > 0) {
            parent = domUtils.splitDomSingle(parentList[0], container);
            domUtils.before(container, parent);
            domUtils.remove(parent);
        }
    },
    fixCode: function (code) {
        if (!code) {
            return null;
        }
        var container = code.parentNode;
        if (domUtils.hasClass(container, CONST.CLASS.CODE_CONTAINER)) {
            return null;
        }
        if (container === ENV.doc.body || container.childNodes.length !== 1) {
            container = ENV.doc.createElement('div');
            domUtils.before(container, code);
            container.appendChild(code);
        }
        return container;
    },
    focusToFirst: function (cm) {
        cm.focus();
        cm.setCursor({line: 0, ch: 0});
    },
    focusToLast: function (cm) {
        cm.focus();
        cm.setCursor({
            line: cm.lastLine(),
            ch: cm.getLineHandle(cm.lastLine()).text.length
        });
    },
    getContainerFromChild: function (target) {
        return domUtils.getParentByClass(target, CONST.CLASS.CODE_CONTAINER, true);
    },
    getToolsFromChild: function (target) {
        return domUtils.getParentByClass(target, CONST.CLASS.CODE_TOOLS, true);
    },
    getContainerList: function () {
        return ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER);
    },
    getCmMode: function (container) {
        var mode = (container.getAttribute('data-mode') || '').toLowerCase();
        var lastMode = '';
        if (container.isNew && ENV.win.localStorage && !ENV.readonly) {
            lastMode = (ENV.win.localStorage.getItem(codeKey.mode) || '').toLowerCase();
        }
        return modeDic[mode] || modeDic[lastMode] || 'JavaScript';
    },
    getCmTheme: function (container) {
        var theme = (container.getAttribute('data-theme') || '').toLowerCase();
        var lastTheme = '';
        if (container.isNew && ENV.win.localStorage && !ENV.readonly) {
            lastTheme = (ENV.win.localStorage.getItem(codeKey.theme) || '').toLowerCase();
        }
        return themeDic[theme] || themeDic[lastTheme] || 'default';
    },
    getCmWrapper: function (container) {
        return container.querySelector('.' + CONST.CLASS.CODE_MIRROR);
    },
    initCodeNoIDE: function (container) {
        if (!container) {
            return;
        }
        codeUtils.removeCmWrapper(container);

        var textarea = container.querySelector('textarea');
        var codeSrc = '', pre, code;
        if (textarea) {
            codeSrc = textarea.textContent;
            pre = ENV.doc.createElement('pre');
            code = ENV.doc.createElement('code');
            pre.appendChild(code);
            code.textContent = codeSrc;
            domUtils.before(pre, textarea);
            domUtils.remove(textarea);
        }
    },
    removeCmWrapper: function (container) {
        // 创建 cm 实例，必须删除原 cm Dom 结构
        // 如果 codeMirror 在 p 内， 恢复时 CodeMirror 的 div 会成为 p 的 nextSibiling，所以只能根据 id 进行删除
        if (!container.id) {
            return;
        }
        var tmpWrapper = ENV.doc.body.querySelector('.' + CONST.CLASS.CODE_MIRROR + '[data-id=' + container.id + ']');
        domUtils.remove(tmpWrapper);
    },
    initCodeMirror: function (container) {
        if (!container) {
            return;
        }
        codeStyle.insertCommon();

        var options, textarea, cm, cmDiv, cmTag,
            mode, theme;
        if (!container.id) {
            container.id = 'wiz_cm_' + new Date().valueOf() + '_' + Math.floor(10000 * Math.random());
        }
        domUtils.addClass(container, CONST.CLASS.CODE_CONTAINER);
        domUtils.attr(container, {'contenteditable': 'false'});

        mode = codeUtils.getCmMode(container);
        theme = codeUtils.getCmTheme(container);
        // console.log(mode + ', ' + theme);
        if (container.codeMirror) {
            cm = container.codeMirror;
            if (ENV.readonly) {
                // 设置 readOnly 为 nocursor 会导致内容无法被复制
                // cm.setOption('readOnly', 'nocursor');
                // cm.setOption('readOnly', true);
                cm.setOption('matchBrackets', false);
                cm.setOption('readOnly', (ENV.client.type.isPhone || ENV.client.type.isPad) ? 'nocursor' : true);
                cm.setOption('styleActiveLine', false);
                cm.getDoc().clearHistory();
            } else {
                cm.setOption('matchBrackets', true);
                cm.setOption('readOnly', false);
                cm.setOption('styleActiveLine', true);
            }
            return;
        }

        codeUtils.removeCmWrapper(container);

        // 防止 万一 data-id 不存在的情况
        while (container.lastChild && !domUtils.isTag(container.lastChild, 'textarea')) {
            container.removeChild(container.lastChild);
        }

        textarea = container.querySelector('textarea');
        // 清理特殊字符
        domUtils.setTextarea(textarea, utils.replaceSpecialChar(textarea.value));
        options = {
            fixedGutter: false,
            indentUnit: 4,
            // lineWrapping: (ENV.client.type.isPhone || ENV.client.type.isPad),
            lineWrapping: true,
            scrollbarStyle: 'null',
            lineNumbers: true,
            matchBrackets: !ENV.readonly,
            mode: CODE_MIRROR.MODE[mode].cm,
            extraKeys: {
                Tab: function (cm) {
                    var spaces = Array(cm.getOption("indentUnit") + 1).join(" ");
                    cm.replaceSelection(spaces);
                },
                'Ctrl-Z': function () {
                    historyUtils.undo();
                },
                'Cmd-Z': function () {
                    historyUtils.undo();
                },
                'Ctrl-Y': function () {
                    historyUtils.redo();
                },
                'Shift-Cmd-Z': function () {
                    historyUtils.redo();
                },
                'Cmd-Y': function () {
                    historyUtils.redo();
                }
            }
        };
        if (ENV.readonly) {
            // options.readOnly = 'nocursor';
            // options.readOnly = true;

            // 移动端 阅读界面必须设置 readonly 为 nocursor，否则会导致点击代码区域出现 键盘 和 光标
            options.readOnly = (ENV.client.type.isPhone || ENV.client.type.isPad) ? 'nocursor' : true;
            options.styleActiveLine = false;
        } else {
            options.readOnly = false;
            options.styleActiveLine = true;
        }

        cm = CodeMirror.fromTextArea(textarea, options);
        container.codeMirror = cm;
        container.mode = mode;
        cm.display.wrapper.setAttribute('data-id', container.id);
        cmDiv = container.querySelector('.' + CONST.CLASS.CODE_MIRROR);
        cmTag = ENV.doc.createElement(CONST.TAG.CODE_MIRROR);
        domUtils.before(cmTag, cmDiv);
        cmTag.appendChild(cmDiv);

        codeUtils.changeMode(container, mode);
        codeUtils.changeTheme(container, theme);

        // keydown & beforeSelectionChange 组合用于 衔接 CodeMirror 与 Editor 的光标移动
        var cursorDirect, lastCursorPos = '';
        cm.on('keydown', function (cm, e) {
            var keyCode = e.keyCode || e.which;
            if (keyCode === 37 || keyCode === 38) {
                // left or up
                cursorDirect = -1;
            } else if (keyCode === 39 || keyCode === 40) {
                // right or down
                cursorDirect = 1;
            } else {
                // other
                cursorDirect = 0;
            }
        });
        cm.on('beforeSelectionChange', function (cm, obj) {
            var range = obj.ranges[0],
                anchor = range.anchor,
                head = range.head,
                newCur;
            var curCursorPos = [anchor.line, anchor.ch, head.line, head.ch].join('.');
            var codeContainer, cm;
            // 选中区间时，不处理光标
            if (obj.origin === "+move" &&
                anchor.line == head.line && anchor.ch == head.ch &&
                curCursorPos === lastCursorPos) {
                if (cursorDirect === -1) {
                    newCur = domUtils.getPreviousNode(container);
                    while (newCur && !domUtils.isTag(newCur, 'br') && domUtils.isEmptyDom(newCur)) {
                        newCur = domUtils.getPreviousNode(newCur);
                    }
                    if (newCur) {
                        codeContainer = codeUtils.getContainerFromChild(newCur);
                        if (codeContainer) {
                            cm = codeContainer.codeMirror;
                            codeUtils.focusToLast(cm);
                        } else {
                            ENV.doc.body.setAttribute('contenteditable', true);
                            rangeUtils.setRange(newCur, domUtils.getEndOffset(newCur));
                        }
                    }
                } else if (cursorDirect === 1) {
                    newCur = domUtils.getNextNode(container);
                    while (newCur && !domUtils.isTag(newCur, 'br') && domUtils.isEmptyDom(newCur)) {
                        newCur = domUtils.getNextNode(newCur);
                    }
                    if (newCur) {
                        codeContainer = codeUtils.getContainerFromChild(newCur);
                        if (codeContainer) {
                            cm = codeContainer.codeMirror;
                            codeUtils.focusToFirst(cm);
                        } else {
                            ENV.doc.body.setAttribute('contenteditable', true);
                            rangeUtils.setRange(newCur, 0);
                        }
                    }
                }
            }

            lastCursorPos = curCursorPos;
        });

        cm.on('beforeChange', function (cm, obj) {
            // undo/redo 不能在 beforeChange 内执行 cancel 会导致 history 清空

            // 捕获 CodeMirror 修改前的事件，相当于 editor 的 KeyDown
            if (obj.origin !== 'redo' && obj.origin !== 'undo') {
                historyUtils.saveSnap(false, {
                    type: CONST.HISTORY.TYPE.CODE_MIRROR,
                    cmContainerId: container.id
                });
            }
        });
        cm.on('change', function (cm, obj) {
            if (/^(redo|undo|paste)$/i.test(obj.origin)) {
                // 修改样式不使用滚动条后，redo/undo/paste 大段代码，CodeMirror 只显示一部分代码，所以必须要重新设置大小
                cm.setSize();
            }
        });
        cm.on('copy', function (cm, e) {
            // 移动端 阅读界面 因为设置了 nocursor 会导致无法正确进行复制，所以 必须忽略 codemirror 内部的复制功能
            if ((ENV.client.type.isPhone || ENV.client.type.isPad) && ENV.readonly) {
                e.codemirrorIgnore = true;
            }
        });

        cm.on('focus', function (cm, obj) {
            var container = codeUtils.getContainerFromChild(cm.display.wrapper);
            codeUtils.initTools(container);

            // 目前已经不显示横向滚动条，全部进行自动换行，所以此功能取消
            // 以后重新开启时，需要注意，此功能会导致 IOS 手机端焦点进入 CodeMirror 后，再无法失去焦点进入其他区域

            // if (!ENV.readonly) {
            //     // Chrome 49 内核 body contenteditable = true 时，
            //     // codeMirror 内的 focus 方法会导致页面跳跃，
            //     // 导致 横向滚动条移动时 页面纵向滚动条会快速移动到光标位置
            //     // 因此 必须 利用 focus 、blur 事件调整 body 的 contenteditable
            //     ENV.doc.body.setAttribute('contenteditable', false);
            // }
            // domUtils.removeClass(cm.display.wrapper, 'CodeMirror-blur');
        });
        cm.on('blur', function (cm, obj) {

            // if (!ENV.readonly) {
            //     ENV.doc.body.setAttribute('contenteditable', true);
            // }
            // domUtils.addClass(cm.display.wrapper, 'CodeMirror-blur');
        });

        // patch
        var sizer = container.querySelector('.CodeMirror-sizer');
        var emptyLine;
        if (sizer) {
            emptyLine = sizer.nextElementSibling;
            if (emptyLine) {
                emptyLine.style.height = '13px';
            }
        }
    },
    initTools: function (container) {
        if (!container || ENV.readonly || ENV.client.type.isPhone || ENV.client.type.isPad) {
            return;
        }

        var cmWrapper = codeUtils.getCmWrapper(container);
        var codeTools = container.querySelector('.' + CONST.CLASS.CODE_TOOLS);
        var modeSelector, themeSelector;
        var mode = codeUtils.getCmMode(container);
        var theme = container.getAttribute('data-theme');
        if (!codeTools) {
            codeTools = ENV.doc.createElement(CONST.TAG.TMP_TAG);
            modeSelector = selectPlugin.create(ModeOptions, CONST.CLASS.CODE_TOOLS_MODE, mode);
            themeSelector = selectPlugin.create(ThemeOptions, CONST.CLASS.CODE_TOOLS_THEME, theme);
            codeTools.appendChild(modeSelector);
            codeTools.appendChild(themeSelector);
            domUtils.addClass(modeSelector, CONST.CLASS.CODE_TOOLS_MODE);
            domUtils.addClass(themeSelector, CONST.CLASS.CODE_TOOLS_THEME);
            domUtils.addClass(codeTools, CONST.CLASS.CODE_TOOLS);
        }
        cmWrapper.appendChild(codeTools);
    },
    insertCodeSrc: function (container, src) {
        var cm = container.codeMirror;
        if (!cm) {
            return;
        }
        cm.replaceSelection(src, 'end');
    },
    // 1. 处理 删除操作
    // 2. 判断从 CodeMirror 之外移动光标是否进入 CodeMirror
    onKeyDown: function (e) {
        var range = rangeUtils.getRange();
        var sel = ENV.doc.getSelection();
        if (!range || !range.collapsed) {
            return false;
        }

        var code = e.keyCode || e.which;
        var startLast = rangeUtils.getRangeDetail(range.startContainer, range.startOffset),
            start, block, target;
        var direct, charMove,
            searchBackward = false,
            searchForward = false,
            searchStart = null,
            searchEnd = null,
            container = null,
            cm;

        /**
         * Backspace
         */
        if (code === 8 && startLast.offset === 0) {
            target = domUtils.getPreviousNode(startLast.container, false);
            container = codeUtils.getContainerFromChild(target);
        }
        /**
         * Delete
         */
        if (code === 46 && (domUtils.isTag(startLast.container, 'br') ||
                startLast.offset === domUtils.getEndOffset(startLast.container))) {
            target = domUtils.getNextNode(startLast.container, false);
            container = codeUtils.getContainerFromChild(target);

            if (container && domUtils.isTag(startLast.container, 'br')) {
                // 删除当前空行
                domUtils.remove(startLast.container);
                codeUtils.focusToFirst(container.codeMirror);
                return true;
            }
        }

        if (container) {
            historyUtils.saveSnap(false);
            domUtils.remove(container);
            return true;
        }

        // 判断键盘，控制光标移动
        switch (code) {
            case 37:
                //left
                if (!e.ctrlKey && !e.metaKey) {
                    charMove = true;
                    sel.modify('move', 'backward', 'character');
                }
                direct = {x: -1, y: 0};
                break;
            case 38:
                //up
                if (!e.ctrlKey && !e.metaKey) {
                    charMove = true;
                    sel.modify('move', 'backward', 'line');
                }
                direct = {x: 0, y: -1};
                break;
            case 39:
                //right
                if (!e.ctrlKey && !e.metaKey) {
                    charMove = true;
                    sel.modify('move', 'forward', 'character');
                }
                direct = {x: 1, y: 0};
                break;
            case 40:
                //down
                if (!e.ctrlKey && !e.metaKey) {
                    charMove = true;
                    sel.modify('move', 'forward', 'line');
                }
                direct = {x: 0, y: 1};
                break;
        }

        // 模拟光标移动后，检查移动范围内是否出现 CodeMirror 的 container
        if (charMove) {
            range = rangeUtils.getRange();
            start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            searchStart = startLast.container;
            searchEnd = start.container;

            if (start.container === startLast.container && start.offset !== startLast.offset) {
                // 光标移动且 start 未变化，说明是在但行内移动，无需查找 CodeMirror Container

            } else if (start.container !== startLast.container) {
                // 光标移动
                if (direct.x > 0 || direct.y > 0) {
                    // forward
                    searchForward = true;
                } else {
                    // backward
                    searchBackward = true;
                }
            } else {
                // 光标未移动
                if (direct.x > 0 || direct.y > 0) {
                    // forward
                    searchForward = true;
                } else {
                    // backward
                    searchBackward = true;
                }
                searchEnd = ENV.doc.body;
            }

            if (searchForward) {
                block = domUtils.getNextNode(searchStart, false, searchEnd);
                while (block && block !== searchEnd && !domUtils.isTag(block, 'br') && domUtils.isEmptyDom(block)) {
                    block = domUtils.getNextNode(block, false, searchEnd);
                }
                if (block) {
                    block = domUtils.getNextBlock(block);
                }
            } else if (searchBackward) {
                block = domUtils.getPreviousNode(searchStart, false, searchEnd);
                while (block && block !== searchEnd && !domUtils.isTag(block, 'br') && domUtils.isEmptyDom(block)) {
                    block = domUtils.getPreviousNode(block, false, searchEnd);
                }
                if (block) {
                    block = domUtils.getPrevBlock(block);
                }
            }

            if (block) {
                container = codeUtils.getContainerFromChild(block);
                cm = container && container.codeMirror;
                if (!cm) {
                    container = null;
                } else {
                    // 找到 container 后，设置对应光标位置
                    cm.focus();
                    if (direct.x > 0 || direct.y > 0) {
                        codeUtils.focusToFirst(cm);
                    } else {
                        codeUtils.focusToLast(cm);
                    }
                }
            }

            // 没有找到 Container 的情况下 恢复光标位置
            if (!container) {
                if (direct.x < 0) {
                    sel.modify('move', 'forward', 'character');
                } else if (direct.y < 0) {
                    sel.modify('move', 'forward', 'line');
                } else if (direct.x > 0) {
                    sel.modify('move', 'backward', 'character');
                } else if (direct.y > 0) {
                    sel.modify('move', 'backward', 'line');
                }
            }
        }
        return !!container;
    },
    saveToText: function () {
        var containerList = codeUtils.getContainerList();
        var container, cm, i;
        for (i = containerList.length - 1; i >= 0; i--) {
            container = containerList[i];
            cm = container.codeMirror;
            if (cm) {
                cm.save();
            }
        }
    },
    pastePatch: {
        fix: function () {
            var codeList = ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER_PASTE);
            var i, container;
            for (i = codeList.length; i >= 0; i--) {
                container = codeList[i];
                domUtils.removeClass(container, CONST.CLASS.CODE_CONTAINER_PASTE);
                codeUtils.fixCodeContainer(container);
            }
        },
        ready: function (dom, isInTable) {
            var codeList = dom.querySelectorAll('.' + CONST.CLASS.CODE_MIRROR);
            var codeContainer, textarea;
            var container, codeMirror, rowList, mode, theme,
                codeIndex, rowIndex, row, codeLine, src;
            for (codeIndex = codeList.length - 1; codeIndex >= 0; codeIndex--) {
                codeMirror = codeList[codeIndex];
                container = codeUtils.getContainerFromChild(codeMirror);
                if (!container) {
                    container = codeMirror;
                }
                codeLine = [];

                rowList = codeMirror.querySelectorAll('.' + CONST.CLASS.CODE_MIRROR_LINE);
                for (rowIndex = 0; rowIndex < rowList.length; rowIndex++) {
                    row = rowList[rowIndex];
                    src = row.innerText || row.textContent;
                    codeLine.push(src.replace(CONST.FILL_CHAR_REG, ''));
                }

                codeContainer = ENV.doc.createElement('div');
                if (isInTable) {
                    codeContainer.innerHTML = '<br/>' + codeLine.join('<br/>') + '<br/><br/>';
                } else {
                    mode = container.getAttribute('data-mode');
                    theme = container.getAttribute('data-theme');

                    textarea = ENV.doc.createElement('textarea');
                    textarea.textContent = codeLine.join('\n');
                    domUtils.addClass(codeContainer, CONST.CLASS.CODE_CONTAINER_PASTE);
                    codeContainer.appendChild(textarea);
                    if (mode) {
                        codeContainer.setAttribute('data-mode', mode);
                    }
                    if (theme) {
                        codeContainer.setAttribute('data-theme', theme);
                    }
                }

                domUtils.before(codeContainer, container);
                domUtils.remove(container);
            }
        }
    },
    oldPatch: {
        oldCodeReady: function() {
            // 分割代码内的其他内容 (A / Img)

            var codeList;
            var code, codeI;

            codeList = ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER_OLD);
            for (codeI = codeList.length - 1; codeI >= 0; codeI--) {
                code = codeList[codeI];
                split(code, 'a');
            }

            // 每一次拆分后必须重新遍历 codeList，因为 code 区块会增加
            codeList = ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER_OLD);
            for (codeI = codeList.length - 1; codeI >= 0; codeI--) {
                code = codeList[codeI];
                split(code, 'img');
            }

            function split(_code, selector) {
                var domList, domI, dom, domP, main;
                var div;
                domList = _code.querySelectorAll(selector);
                for (domI = domList.length - 1; domI >= 0; domI--) {
                    dom = domList[domI];
                    // a 标签内如果 text 为空，才进行处理（只有 a 里面是图片 才分割）
                    if (!utils.isEmpty(dom.innerText)) {
                        continue;
                    }
                    main = domUtils.getParentByClass(dom, CONST.CLASS.CODE_CONTAINER_OLD, false);
                    if (main) {
                        domP = domUtils.splitDomSingle(main, dom);
                        div = ENV.doc.createElement('div');
                        div.appendChild(dom);
                        domUtils.before(div, domP);
                        domUtils.remove(domP);
                    }
                }
            }
        },
        fixOldCode: function () {
            var codeList, code, mode,
                container, src, textarea,
                i;
            var result = [];

            codeUtils.oldPatch.oldCodeReady();
            codeList = ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER_OLD);
            if (codeList.length > 50) {
                // 如果代码段大于 50 不进行修正，会导致页面性能大幅下降
                return result;
            }

            for (i = codeList.length - 1; i >= 0; i--) {
                code = codeList[i];
                container = codeUtils.fixCode(code);
                src = codeUtils.oldPatch.getSrc(code);
                mode = codeUtils.oldPatch.getMode(code);
                container.removeChild(code);
                textarea = ENV.doc.createElement('textarea');
                textarea.value = src;
                textarea.style.display = 'none';
                container.appendChild(textarea);
                domUtils.addClass(container, CONST.CLASS.CODE_CONTAINER);
                if (mode) {
                    domUtils.attr(container, {
                        'data-mode': mode
                    });
                }
                result.push(container);
            }
            return result;
        },
        getMode: function (code) {
            var regCode = /language-([\w]+)/i;
            var regPre = /lang-([\w]+)/i;
            var modeObj = code.querySelector('code'), result,
                mode;
            if (modeObj && modeObj.className && (result = modeObj.className.match(regCode))) {
                mode = result[1];
            } else if (code.className && (result = code.className.match(regPre))) {
                mode = result[1];
            }
            if (mode) {
                mode = modeDic[mode.toLowerCase()];
            }
            return mode;
        },
        getSrc: function (code) {
            if (!code) {
                return '';
            }
            // 注意：
            // 1、旧的笔记代码段没有严格控制，所以会导致各种元素穿插在内（例如：超链接、图片等）
            // 2、prettyprint 渲染后 有的代码段 空行是 <code></code> 直接使用 innerText 获取源码 会导致空行丢失，必须修正

            var domList = code.querySelectorAll('code');
            var dom, i, j;
            // 修正空行
            for (i = 0, j = domList.length; i < j; i++) {
                dom = domList[i];
                if (!dom.innerText) {
                    dom.innerText = '\n';
                }
            }

            // 列表后面只要有内容， innerText 在结尾就会多出一个 \n，如果不处理 在 CodeMirror 中就会多出一行
            return (code.innerText || code.textContent || '').replace(/\n$/, '');
        }

    },
    highlight: {
        clear: function (container) {
            var cm = container.codeMirror;
            if (!cm) {
                return;
            }
            var state = container.state;
            if (!state) {
                state = {};
                container.state = state;
            }

            cm.operation(function () {
                state.lastQuery = state.query || null;
                if (!state.query) {
                    return;
                }
                state.query = state.queryText = null;
                cm.removeOverlay(state.overlay);
            });
        },
        clearAll: function() {
            var containerList = codeUtils.getContainerList();
            var i, container;
            for (i=0; i<containerList.length; i++) {
                container = containerList[i];
                codeUtils.highlight.clear(container);
            }
        },
        overlay: function (query) {
            query = new RegExp(query.replace(/[\-\[\]\/\{\}\(\)\*\+\?\.\\\^\$\|]/g, "\\$&"), "gi");
            return {
                token: function (stream) {
                    query.lastIndex = stream.pos;
                    var match = query.exec(stream.string);
                    if (match && match.index === stream.pos) {
                        stream.pos += match[0].length || 1;
                        return "searching";
                    } else if (match) {
                        stream.pos = match.index;
                    } else {
                        stream.skipToEnd();
                    }
                }
            };
        },
        search: function  (container, query) {
            // 如果 CodeMirror 区域内已经处理过同样的关键字，则不要再处理，
            // 否则会导致 highlightUtils 代码 while (ENV.win.find(key, false, true)) {...} 部分出现死循环
            if (container.state && container.state.query &&
                container.state.query.toLowerCase() === query.toLowerCase()) {
                return;
            }
            
            var cm = container.codeMirror;
            if (!cm) {
                return;
            }
            codeUtils.highlight.clear(container);
            var state = container.state;
            query = query.toLowerCase();
            state.query = query;
            cm.removeOverlay(state.overlay);
            state.overlay = codeUtils.highlight.overlay(state.query);
            cm.addOverlay(state.overlay);
        }
    }
};

module.exports = codeUtils;

},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../domUtils/selectPlugin":30,"../rangeUtils/rangeExtend":49,"./codeStyle":15}],17:[function(require,module,exports){
/*
 * $Id: base64.js,v 2.15 2014/04/05 12:58:57 dankogai Exp dankogai $
 *  https://github.com/dankogai/js-base64
 *  Licensed under the MIT license.
 *    http://opensource.org/licenses/mit-license
 *
 *  References:
 *    http://en.wikipedia.org/wiki/Base64
 */


'use strict';
var global = {};
// existing version for noConflict()
var _Base64 = global.Base64;
var version = "2.1.8";
// if node.js, we use Buffer
var buffer;
if (typeof module !== 'undefined' && module.exports) {
    buffer = require('buffer').Buffer;
}
// constants
var b64chars
    = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/';
var b64tab = function (bin) {
    var t = {};
    for (var i = 0, l = bin.length; i < l; i++) t[bin.charAt(i)] = i;
    return t;
}(b64chars);
var fromCharCode = String.fromCharCode;
// encoder stuff
var cb_utob = function (c) {
    if (c.length < 2) {
        var cc = c.charCodeAt(0);
        return cc < 0x80 ? c
            : cc < 0x800 ? (fromCharCode(0xc0 | (cc >>> 6))
        + fromCharCode(0x80 | (cc & 0x3f)))
            : (fromCharCode(0xe0 | ((cc >>> 12) & 0x0f))
        + fromCharCode(0x80 | ((cc >>> 6) & 0x3f))
        + fromCharCode(0x80 | ( cc & 0x3f)));
    } else {
        var cc = 0x10000
            + (c.charCodeAt(0) - 0xD800) * 0x400
            + (c.charCodeAt(1) - 0xDC00);
        return (fromCharCode(0xf0 | ((cc >>> 18) & 0x07))
        + fromCharCode(0x80 | ((cc >>> 12) & 0x3f))
        + fromCharCode(0x80 | ((cc >>> 6) & 0x3f))
        + fromCharCode(0x80 | ( cc & 0x3f)));
    }
};
var re_utob = /[\uD800-\uDBFF][\uDC00-\uDFFFF]|[^\x00-\x7F]/g;
var utob = function (u) {
    return u.replace(re_utob, cb_utob);
};
var cb_encode = function (ccc) {
    var padlen = [0, 2, 1][ccc.length % 3],
        ord = ccc.charCodeAt(0) << 16
            | ((ccc.length > 1 ? ccc.charCodeAt(1) : 0) << 8)
            | ((ccc.length > 2 ? ccc.charCodeAt(2) : 0)),
        chars = [
            b64chars.charAt(ord >>> 18),
            b64chars.charAt((ord >>> 12) & 63),
            padlen >= 2 ? '=' : b64chars.charAt((ord >>> 6) & 63),
            padlen >= 1 ? '=' : b64chars.charAt(ord & 63)
        ];
    return chars.join('');
};
var btoa = global.btoa ? function (b) {
    return global.btoa(b);
} : function (b) {
    return b.replace(/[\s\S]{1,3}/g, cb_encode);
};
var _encode = buffer ? function (u) {
        return (u.constructor === buffer.constructor ? u : new buffer(u))
            .toString('base64')
    }
        : function (u) {
        return btoa(utob(u))
    }
    ;
var encode = function (u, urisafe) {
    return !urisafe
        ? _encode(String(u))
        : _encode(String(u)).replace(/[+\/]/g, function (m0) {
        return m0 == '+' ? '-' : '_';
    }).replace(/=/g, '');
};
var encodeURI = function (u) {
    return encode(u, true)
};
// decoder stuff
var re_btou = new RegExp([
    '[\xC0-\xDF][\x80-\xBF]',
    '[\xE0-\xEF][\x80-\xBF]{2}',
    '[\xF0-\xF7][\x80-\xBF]{3}'
].join('|'), 'g');
var cb_btou = function (cccc) {
    switch (cccc.length) {
        case 4:
            var cp = ((0x07 & cccc.charCodeAt(0)) << 18)
                    | ((0x3f & cccc.charCodeAt(1)) << 12)
                    | ((0x3f & cccc.charCodeAt(2)) << 6)
                    | (0x3f & cccc.charCodeAt(3)),
                offset = cp - 0x10000;
            return (fromCharCode((offset >>> 10) + 0xD800)
            + fromCharCode((offset & 0x3FF) + 0xDC00));
        case 3:
            return fromCharCode(
                ((0x0f & cccc.charCodeAt(0)) << 12)
                | ((0x3f & cccc.charCodeAt(1)) << 6)
                | (0x3f & cccc.charCodeAt(2))
            );
        default:
            return fromCharCode(
                ((0x1f & cccc.charCodeAt(0)) << 6)
                | (0x3f & cccc.charCodeAt(1))
            );
    }
};
var btou = function (b) {
    return b.replace(re_btou, cb_btou);
};
var cb_decode = function (cccc) {
    var len = cccc.length,
        padlen = len % 4,
        n = (len > 0 ? b64tab[cccc.charAt(0)] << 18 : 0)
            | (len > 1 ? b64tab[cccc.charAt(1)] << 12 : 0)
            | (len > 2 ? b64tab[cccc.charAt(2)] << 6 : 0)
            | (len > 3 ? b64tab[cccc.charAt(3)] : 0),
        chars = [
            fromCharCode(n >>> 16),
            fromCharCode((n >>> 8) & 0xff),
            fromCharCode(n & 0xff)
        ];
    chars.length -= [0, 0, 2, 1][padlen];
    return chars.join('');
};
var atob = global.atob ? function (a) {
    return global.atob(a);
} : function (a) {
    return a.replace(/[\s\S]{1,4}/g, cb_decode);
};
var _decode = buffer ? function (a) {
    return (a.constructor === buffer.constructor
        ? a : new buffer(a, 'base64')).toString();
}
    : function (a) {
    return btou(atob(a))
};
var decode = function (a) {
    return _decode(
        String(a).replace(/[-_]/g, function (m0) {
                return m0 == '-' ? '+' : '/'
            })
            .replace(/[^A-Za-z0-9\+\/]/g, '')
    );
};
var noConflict = function () {
    var Base64 = global.Base64;
    global.Base64 = _Base64;
    return Base64;
};
// export Base64
global.Base64 = {
    VERSION: version,
    atob: atob,
    btoa: btoa,
    fromBase64: decode,
    toBase64: encode,
    utob: utob,
    encode: encode,
    encodeURI: encodeURI,
    btou: btou,
    decode: decode,
    noConflict: noConflict
};
// if ES5 is available, make Base64.extendString() available
if (typeof Object.defineProperty === 'function') {
    var noEnum = function (v) {
        return {value: v, enumerable: false, writable: true, configurable: true};
    };
    global.Base64.extendString = function () {
        Object.defineProperty(
            String.prototype, 'fromBase64', noEnum(function () {
                return decode(this)
            }));
        Object.defineProperty(
            String.prototype, 'toBase64', noEnum(function (urisafe) {
                return encode(this, urisafe)
            }));
        Object.defineProperty(
            String.prototype, 'toBase64URI', noEnum(function () {
                return encode(this, true)
            }));
    };
}

module.exports = global.Base64;
},{"buffer":2}],18:[function(require,module,exports){
/**
 * 内部使用的标准常量.
 */

var FILL_CHAR = '\u200B';
var CONST = {
    //在此间隔内修订，不生成新的 span，只修改修订时间
    AMEND_TIME_SPACE: 3 * 60 * 1000, // 3分钟
    //在此间隔内修订的内容， 被当作同一批次修订，批量拒绝或接受
    AMEND_BATCH_TIME_SPACE: 30 * 1000, // 30秒
    //判断是否正在进行中文输入法的标识（true 为正在进行中...）
    COMPOSITION_START: false,
    //String.fromCharCode(8203)
    FILL_CHAR: FILL_CHAR,
    FILL_CHAR_REG: new RegExp(FILL_CHAR, 'ig'),
    AMEND: {
        INFO_SPACE: 0, //修订信息图层与目标间隔
        INFO_TIMER: 300 //修订timer 间隔
    },
    ATTR: {
        IMG: 'data-wiz-img',
        IMG_MASK: 'data-wiz-img-mask',
        IMG_RATE: 'data-wiz-img-rate',
        IMG_EDITING: 'data-wiz-img-editing',
        SPAN: 'data-wiz-span',
        SPAN_USERID: 'data-wiz-user-id',
        SPAN_INSERT: 'data-wiz-insert',
        SPAN_DELETE: 'data-wiz-delete',
        SPAN_PASTE: 'data-wiz-paste',
        SPAN_PASTE_TYPE: 'data-wiz-paste-type',
        SPAN_PASTE_ID: 'data-wiz-paste-id',
        SPAN_TIMESTAMP: 'data-wiz-amend-time',
        TODO_ID: 'wiz_todo_id',
        TODO_CHECK: 'data-wiz-check'
    },
    CLASS: {
        BLOCK_SCROLL: 'wiz-block-scroll',
        CODE_CONTAINER: 'wiz-code-container',
        CODE_CONTAINER_PASTE: 'wiz-code-container-paste',
        CODE_CONTAINER_OLD: 'prettyprint',
        CODE_MIRROR: 'CodeMirror',
        CODE_MIRROR_LINE: 'CodeMirror-line',
        CODE_MIRROR_MEASURE: 'CodeMirror-measure',
        CODE_MIRROR_GUTTER: 'CodeMirror-gutter-wrapper',
        CODE_MIRROR_HSCROLL: 'CodeMirror-scroll',
        CODE_TOOLS: 'wiz-code-tools',
        CODE_TOOLS_MODE: 'wiz-code-tools-mode',
        CODE_TOOLS_THEME: 'wiz-code-tools-theme',
        IMG_ATTACHMENT: 'wiz-img-attachment',
        IMG_NOT_DRAG: 'wiz-img-cannot-drag',
        IMG_RESIZE_ACTIVE: 'wiz-img-resize-active',
        IMG_RESIZE_CONTAINER: 'wiz-img-resize-container',
        IMG_RESIZE_HANDLE: 'wiz-img-resize-handle',
        ORDER_LIST_LEVEL: ['wiz-list-level3', 'wiz-list-level1', 'wiz-list-level2'],
        SELECTED_CELL: 'wiz-selected-cell',
        SELECTED_CELL_MULTI: 'wiz-selected-cell-multi',
        SELECT_PLUGIN_CONTAINER: 'wiz-select-plugin-container',
        SELECT_PLUGIN_HEADER: 'wiz-select-plugin-header',
        SELECT_PLUGIN_HEADER_TEXT: 'wiz-select-plugin-header-text',
        SELECT_PLUGIN_OPTIONS: 'wiz-select-plugin-options',
        SELECT_PLUGIN_OPTIONS_ITEM: 'wiz-select-plugin-options-item',
        TABLE_CONTAINER: 'wiz-table-container',
        TABLE_TOOLS: 'wiz-table-tools',
        TABLE_BODY: 'wiz-table-body',
        TABLE_MENU_BUTTON: 'wiz-table-menu-button',
        TABLE_MENU_ITEM: 'wiz-table-menu-item',
        TABLE_MENU_SUB: 'wiz-table-menu-sub',
        TABLE_MOVING: 'wiz-table-moving',
        TODO_ACCOUNT: 'wiz-todo-account',
        TODO_AVATAR: 'wiz-todo-avatar',
        TODO_CHECKBOX: 'wiz-todo-checkbox',
        TODO_CHECK_IMG_OLD: 'wiz-todo-img',
        TODO_DATE: 'wiz-todo-dt',
        TODO_LAYER: 'wiz-todo-layer',
        TODO_MAIN: 'wiz-todo-main',
        TODO_LABEL_OLD: 'wiz-todo-label',
        TODO_CHECKED: 'wiz-todo-checked',
        TODO_UNCHECKED: 'wiz-todo-unchecked',
        TODO_CHECKED_OLD: 'wiz-todo-label-checked',
        TODO_UNCHECKED_OLD: 'wiz-todo-label-unchecked',
        TODO_TAIL_OLD: 'wiz-todo-tail', //新版本 todoList 取消此元素
        TODO_USER_AVATAR: 'wiz-todo-avatar-',
        TODO_USER_INFO: 'wiz-todo-completed-info'
    },
    //客户端相关的事件定义
    CLIENT_EVENT: {
        WizEditorPaste: 'wizEditorPaste',
        WizEditorClickImg: 'wizEditorClickImg',
        WizReaderClickImg: 'wizReaderClickImg',
        WizMarkdownRender: 'wizMarkdownRender',
        WizEditorTrackEvent: 'wizEditorTrackEvent',
        // WizGetWebViewSize: 'wizGetWebViewSize'
    },
    COLOR: [
        '#CB3C3C', '#0C9460', '#FF3399', '#FF6005', '#8058BD', '#009999', '#8AA725',
        '#339900', '#CC6600', '#3BBABA', '#D4CA1A', '#2389B0', '#006699', '#FF8300',
        '#2C6ED5', '#FF0000', '#B07CFF', '#CC3399', '#EB4847', '#3917E6'
    ],
    CSS: {
        IMG: {
            SPAN: {
                position: 'relative',
                display: 'inline-block'
            },
            MASK: {
                position: 'absolute',
                width: '100% !important',
                height: '100% !important',
                top: '0',
                left: '0',
                opacity: '.5',
                filter: 'alpha(opacity=50)',
                border: '2px solid',
                'box-sizing': 'border-box',
                '-webkit-box-sizing': 'border-box',
                '-moz-box-sizing': 'border-box'
            }
        },
        IMG_DELETED: {
            background: '#fdc6c6 url(data:image/gif;base64,R0lGODlhDwAPAIABAIcUFP///yH5BAEKAAEALAAAAAAPAA8AAAIajI8IybadHjxyhjox1I0zH1mU6JCXCSpmUAAAOw==)',
            'border-color': '#E47070'
        },
        IMG_INSERT: {
            background: '#ccffcc',
            'border-color': '#00AA00'
        },
        TODO_LIST: {
            IMG_WIDTH: 40
        },
        Z_INDEX: {
            amendInfo: 150,
            tableBorder: 105,
            tableColRowLine: 120,
            tableRangeDot: 110,
            tableTDBefore: 100,
            tableTools: 130,
            tableToolsArrow: 10
        }
    },
    //全局事件 id 集合
    EVENT: {
        BEFORE_GET_DOCHTML: 'BEFORE_GET_DOCHTML',
        BEFORE_SAVESNAP: 'BEFORE_SAVESNAP',
        AFTER_INSERT_DOM: 'AFTER_INSERT_DOM',
        AFTER_RESTORE_HISTORY: 'AFTER_RESTORE_HISTORY',
        EXEC_COMMEND: 'EXEC_COMMEND',

        ON_DBLCLICK: 'ON_DBLCLICK',
        ON_CLICK: 'ON_CLICK',
        ON_COMPOSITION_START: 'ON_COMPOSITION_START',
        ON_COMPOSITION_END: 'ON_COMPOSITION_END',
        ON_COPY: 'ON_COPY',
        ON_CUT: 'ON_CUT',
        ON_DOM_SUBTREE_MODIFIED: 'ON_DOM_SUBTREE_MODIFIED',
        ON_DRAG_START: 'ON_DRAG_START',
        ON_DRAG_ENTER: 'ON_DRAG_ENTER',
        ON_DROP: 'ON_DROP',
        ON_EXEC_COMMAND: 'ON_EXEC_COMMAND',
        ON_KEY_DOWN: 'ON_KEY_DOWN',
        ON_KEY_UP: 'ON_KEY_UP',
        ON_MOUSE_DOWN: 'ON_MOUSE_DOWN',
        ON_MOUSE_MOVE: 'ON_MOUSE_MOVE',
        ON_MOUSE_OVER: 'ON_MOUSE_OVER',
        ON_MOUSE_UP: 'ON_MOUSE_UP',
        ON_ORIENTATION_CHANGE: 'ON_ORIENTATION_CHANGE',
        ON_PASTE: 'ON_PASTE',
        ON_SCROLL: 'ON_SCROLL',
        ON_SELECT_PLUGIN_CHANGE: 'ON_SELECT_PLUGIN_CHANGE',
        ON_SELECTION_CHANGE: 'ON_SELECTION_CHANGE',
        ON_SELECT_START: 'ON_SELECT_START',
        ON_TOUCH_START: 'ON_TOUCH_START',
        ON_TOUCH_END: 'ON_TOUCH_END',
        UPDATE_RENDER: 'UPDATE_RENDER'
    },
    HISTORY: {
        TYPE: {
            CODE_MIRROR: 'CODE_MIRROR',
            COMMON: 'COMMON'
        }
    },
    ID: {
        AMEND_INFO: 'wiz-amend-info',
        AMEND_INFO_SINGLE: 'wiz-amend-info-single',
        AMEND_INFO_MULTI: 'wiz-amend-info-multi',
        AMEND_INFO_NAME: 'wiz-amend-info-name',
        AMEND_INFO_IMG: 'wiz-amend-info-image',
        AMEND_INFO_CONTENT: 'wiz-amend-info-content',
        AMEND_INFO_TIME: 'wiz-amend-info-time',
        AMEND_INFO_TOOLS: 'wiz-amend-info-tools',
        AMEND_INFO_ACCEPT: 'wiz-amend-info-accept',
        AMEND_INFO_REFUSE: 'wiz-amend-info-refuse',
        AMEND_USER_INFO: 'wiz-amend-user',
        CODE_STYLE: 'wiz_code_style',
        FORMAT_PAINTER_STYLE: 'wiz_format_painter_style',
        IFRAME_FOR_SAVE: 'wiz-iframe-for-save',
        TABLE_RANGE_BORDER: 'wiz-table-range-border',
        TABLE_ROW_LINE: 'wiz-table-row-line',
        TABLE_COL_LINE: 'wiz-table-col-line',
        TODO_STYLE: 'wiz_todo_style',
        TODO_STYLE_OLD: 'wiz_todo_style_id',
        TODO_AVATAR_STYLE: 'wiz_todo_style_avatar_',
        WIZ_DEFAULT_STYLE: 'wiz_custom_css'
    },
    NAME: {
        // NO_ABSTRACT_START: 'Document-Abstract-Start',
        // NO_ABSTRACT_END: 'Document-Abstract-End',
        CODE_STYLE: 'wiz_code_style',
        STYLE_FOR_LOAD: 'wiz_style_for_load',
        TMP_STYLE: 'wiz_tmp_editor_style',
        UNSAVE_STYLE: 'wiz_unsave_style'
    },
    NOTE_TYPE: {
        COMMON: 'common',
        MARKDOWN: 'markdown',
        MATHJAX: 'mathjax'
    },
    TAG: {
        CODE_MIRROR: 'wiz_code_mirror',
        TMP_TAG: 'wiz_tmp_tag',
        TMP_PLUGIN_TAG: 'wiz_tmp_plugin_tag',
        TMP_HIGHLIGHT_TAG: 'wiz_tmp_highlight_tag'
    },
    TYPE: {
        IMG_DELETE: 'delete',
        IMG_INSERT: 'insert',
        PASTE: {
            START: 'start',
            END: 'end',
            CONTENT: 'content'
        },
        POS: {
            upLeft: 'up-left',
            downLeft: 'down-left',
            leftUp: 'left-up',
            rightUp: 'right-up',
            upRight: 'up-right',
            downRight: 'down-right',
            leftDown: 'left-down',
            rightDown: 'right-down'
        },
        TABLE: {
            COPY: 'copy',
            PASTE: 'paste',
            CLEAR_CELL: 'clearCell',
            MERGE_CELL: 'mergeCell',
            SPLIT_CELL: 'splitCell',
            INSERT_ROW_UP: 'insertRowUp',
            INSERT_ROW_DOWN: 'insertRowDown',
            INSERT_COL_LEFT: 'insertColLeft',
            INSERT_COL_RIGHT: 'insertColRight',
            DELETE_ROW: 'deleteRow',
            DELETE_COL: 'deleteCol',
            SET_CELL_BG: 'setCellBg',
            SET_CELL_ALIGN: 'setCellAlign',
            DISTRIBUTE_COLS: 'distributeCols',
            DELETE_TABLE: 'deleteTable'
        },
        TODO: 'tasklist'
    }
};

module.exports = CONST;
},{}],19:[function(require,module,exports){
/**
 * 依赖的 css && 非可打包的 js 文件加载控制
 */
var utils = require('./utils'),
    scriptLoader = require('./scriptLoader');

function loadGroup(doc, group, callback) {
    scriptLoader.load(doc, group, callback);
}

function makeCallback(doc, loadFiles, callback) {
    var count = 0, max = loadFiles.length;

    var cb = function () {
        if (count < max) {
            loadGroup(doc, loadFiles[count++], cb);
        } else if (callback) {
            callback();
        }
    };

    return cb;
}

var dependLoader = {
    getDependencyFiles: function (envDependency, type, id) {
        var i, j, g, ii, jj, gg, group;
        var fileList = [];
        for (i = 0, j = envDependency[type][id].length; i < j; i++) {
            g = envDependency[type][id][i];
            if (type == 'css') {
                fileList.push(envDependency.files[type][g]);
            } else {
                group = [];
                for (ii = 0, jj = g.length; ii < jj; ii++) {
                    gg = g[ii];
                    group.push(envDependency.files[type][gg]);
                }
                fileList.push(group);
            }
        }
        return fileList;
    },
    loadJs: function (doc, loadFiles, callback) {
        var cb = makeCallback(doc, loadFiles, callback);
        cb();
    },
    loadCss: function (doc, loadFiles) {
        var i, j;
        for (i = 0, j = loadFiles.length; i < j; i++) {
            utils.loadSingleCss(doc, loadFiles[i]);
        }
    }
};

module.exports = dependLoader;
},{"./scriptLoader":23,"./utils":24}],20:[function(require,module,exports){
/**
 * wizEditor 环境参数，保存当前 document 等
 */
var CONST = require('./const'),
    Lang = require('./lang'),
    base64 = require('./base64'),
    initLang = Lang.initLang;

var GlobalEvent = {};
var WizNotCmdInditify = 'wiznotecmd://';

var ENV = {
    options: {
        document: null,
        autoFocus: true, //only for editor
        lang: 'en',
        clientType: '',
        noteType: 'common',
        userInfo: null,
        usersData: [],
        userNameEncoder: '',
        dependencyUrl: '',
        indexFilesPath: '',
        indexFilesFullPath: '',
        maxRedo: 100,    //only for editor
        noAmend: false,  //only for read
        codeNoIDE: false,  //only for read
        table: {         //only for editor
            colWidth: 120,      //默认列宽
            colWidthMin: 30,    //最小列宽
            rowHeightMin: 33    //最小行高
        },
        timeout: {       //only for read
            markdown: 30 * 1000,
            mathJax: 30 * 1000
        },
        callback: {
            markdown: null, //only for read
            mathJax: null,  //only for read
            redo: null      //only for editor
        },
        // ios: {
        //     webViewHeight: 0, // ios 编辑时 webView 的高度
        //     toolbarHeight: 0  // ios 编辑时 webView 最上面工具栏的高度
        // },
        pc: {
            pluginModified: false
        }
    },
    init: function (type, _options) {
        setOptions(ENV.options, _options);

        var doc = ENV.options.document || window.document;
        var i, user;

        ENV.doc = doc;
        ENV.win = ENV.doc.defaultView;
        initLang(ENV.options.lang);
        ENV.client.setType(ENV.options.clientType);

        // 修正 旧bug 造成的 body 错误 class：
        var bodyClassList = ENV.doc.body.className.split(' ');
        var bodyClass;
        for (i = bodyClassList.length - 1; i >= 0; i--) {
            bodyClass = bodyClassList[i];
            if (!bodyClass || /^wiz-/ig.test(bodyClass)) {
                //删除所有 wiz- 开头的 class
                bodyClassList.splice(i, 1);
            }
        }
        ENV.doc.body.className = bodyClassList.join(' ');

        var filePath = decodeURIComponent(location.pathname),
            filePathStart = filePath.lastIndexOf('/') + 1,
            filePathEnd = filePath.lastIndexOf('.');
        ENV.options.indexFilesPath = filePath.substring(filePathStart, filePathEnd) + '_files';
        ENV.options.indexFilesFullPath = location.protocol + '//' + filePath.substr(0, filePathStart) + ENV.options.indexFilesPath + '/';

        if (ENV.options.userNameEncoder === 'base64') {
            user = ENV.options.userInfo;
            if (user && user.user_name) {
                user.user_name = base64.decode(user.user_name);
            }
            if (ENV.options.userData) {
                for (i = 0; i < ENV.options.usersData.length; i++) {
                    user = ENV.options.usersData[i];
                    user.user_name = base64.decode(user.user_name);
                }
            }
        }

        function setOptions (old, newOptions) {
            if (!newOptions) {
                return;
            }
            var k;
            for (k in old) {
                if (old.hasOwnProperty(k) && typeof newOptions[k] != 'undefined') {
                    if (/^(table|timeout|callback)$/.test(k)) {
                        setOptions(old[k], newOptions[k]);
                    } else {
                        old[k] = newOptions[k];
                    }
                }
            }
        }
    },
    win: null,
    doc: null,
    readonly: null,
    dependency: {
        files: {
            css: {
                fonts: '{dependencyUrl}/fonts.css',
                github2: '{dependencyUrl}/github2.css',
                wizToc: '{dependencyUrl}/wizToc.css'
            },
            cursor: {
                formatPainter: '{dependencyUrl}/format-painter-wiz.cur',
            },
            js: {
                codemirror: '{dependencyUrl}/codemirror/codemirror.js',
                cmActiveLine: '{dependencyUrl}/codemirror/addon/selection/active-line.js',
                cmMatchBrackets: '{dependencyUrl}/codemirror/addon/edit/matchbrackets.js',
                cmMode: '{dependencyUrl}/codemirror/mode/mode.js',
                flowchart: '{dependencyUrl}/flowchart.js',
                jquery: '{dependencyUrl}/jquery-1.11.3.js',
                mathJax: '{dependencyUrl}/mathjax/MathJax.js?config=TeX-AMS-MML_SVG',
                prettify: '{dependencyUrl}/prettify.js',
                raphael: '{dependencyUrl}/raphael.js',
                sequence: '{dependencyUrl}/sequence-diagram.js',
                underscore: '{dependencyUrl}/underscore.js'
            },
            init: function () {
                var dependencyUrl = ENV.options.dependencyUrl.replace(/\\/g, '/');

                _filter(ENV.dependency.files.css);
                _filter(ENV.dependency.files.cursor);
                _filter(ENV.dependency.files.js);

                function _filter (map) {
                    var k;
                    for (k in map) {
                        if (map.hasOwnProperty(k)) {
                            map[k] = map[k].replace('{dependencyUrl}', dependencyUrl);
                        }
                    }
                }
            }
        },
        css: {
            fonts: ['fonts'],
            markdown: ['github2', 'wizToc'],
        },
        js: {
            codeMirror: [
                ['codemirror'],
                ['cmActiveLine', 'cmMatchBrackets', 'cmMode'],
            ],
            markdown: [
                ['jquery'],
                ['prettify', 'raphael', 'underscore'],
                // ['raphael', 'underscore'],
                ['flowchart', 'sequence']
            ],
            mathJax: [
                ['jquery'],
                ['mathJax']
            ]
        }
    },
    /**
     * 客户端类型 & 功能设置
     */
    client: {
        type: {
            isWeb: (function () {
                return (location && location.protocol.indexOf('http') === 0)
            })(),
            isWin: false,
            isMac: false,
            isLinux: false,
            isIOS: false,
            isAndroid: false,
            isPad: false,
            isPhone: false
        },
        sendCmdToWiznote: function () {
        },
        setType: function (type) {
            if (!type) {
                return;
            }
            type = type.toLowerCase();
            if (type.indexOf('windows') > -1) {
                ENV.client.type.isWin = true;
                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    if (cmd == CONST.CLIENT_EVENT.WizReaderClickImg) {
                        ENV.win.external.OnClickedImage(options.src, JSON.stringify(options.imgList));
                    }
                }
            } else if (type.indexOf('ios') > -1) {
                ENV.client.type.isIOS = true;
                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    var url;
                    if (cmd == CONST.CLIENT_EVENT.WizReaderClickImg) {
                        delete options.imgList;
                    }

                    url = WizNotCmdInditify + cmd;
                    var k, params = [];
                    if (!!options) {
                        for (k in options) {
                            if (options.hasOwnProperty(k)) {
                                params.push(k + '=' + encodeURIComponent(options[k]));
                            }
                        }
                        url += '?' + params.join('&');
                    }

                    var tmpTag = ENV.doc.createElement(CONST.TAG.TMP_TAG);
                    var iframe = ENV.doc.createElement('iframe');
                    iframe.setAttribute('src', url);
                    tmpTag.style.display = 'none';
                    tmpTag.appendChild(iframe);
                    ENV.doc.body.appendChild(tmpTag);
                    tmpTag.parentNode.removeChild(tmpTag);
                    iframe = null;
                    tmpTag = null;
                }

            } else if (type.indexOf('android') > -1) {
                ENV.client.type.isAndroid = true;
                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    if (cmd == CONST.CLIENT_EVENT.WizReaderClickImg) {
                        ENV.win.WizNote.onClickImg(options.src, options.imgList.join(','));
                    } else if (cmd == CONST.CLIENT_EVENT.WizEditorClickImg) {
                        ENV.win.WizNote.onEditorClickImage(options.src);
                    }
                }

            } else if (type.indexOf('mac') > -1) {
                var ua = ENV.win.navigator.userAgent;
                if (!/Mac/i.test(ua)) {
                    ENV.client.type.isLinux = true;
                }
                ENV.client.type.isMac = true;

                ENV.client.sendCmdToWiznote = function (cmd, options) {
                    if (cmd == CONST.CLIENT_EVENT.WizReaderClickImg) {
                        ENV.win.WizExplorerApp.onClickedImage(options.src, JSON.stringify(options.imgList));
                    }
                }
            }

            if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
                if (type.indexOf('pad') > -1) {
                    ENV.client.type.isPad = true;
                } else {
                    ENV.client.type.isPhone = true;
                }
            }
        }
    },
    event: {
        add: function (eventId, fun) {
            if (!eventId || !fun || checkFun(eventId, fun)) {
                return;
            }
            var eList = GlobalEvent[eventId];
            if (!eList) {
                eList = [];
            }
            eList.push(fun);
            GlobalEvent[eventId] = eList;

            function checkFun (eventId, fun) {
                if (!eventId || !fun) {
                    return false;
                }
                var i, j,
                    eList = GlobalEvent[eventId];

                if (!eList || eList.length === 0) {
                    return false;
                }
                for (i = 0, j = eList.length; i < j; i++) {
                    if (eList[i] === fun) {
                        return true;
                    }
                }
                return false;
            }
        },
        call: function (eventId) {
            var i, j,
                args = [],
                eList = GlobalEvent[eventId];

            if (!eList || eList.length === 0) {
                return;
            }
            for (i = 1, j = arguments.length; i < j; i++) {
                args.push(arguments[i]);
            }
            for (i = 0, j = eList.length; i < j; i++) {
                eList[i].apply(this, args);
            }
        },
        remove: function (eventId, fun) {
            if (!eventId || !fun) {
                return;
            }
            var i, j,
                eList = GlobalEvent[eventId];

            if (!eList || eList.length === 0) {
                return;
            }
            for (i = 0, j = eList.length; i < j; i++) {
                if (eList[i] === fun) {
                    eList.splice(i, 1);
                }
            }
        }
    }
};

module.exports = ENV;

},{"./base64":17,"./const":18,"./lang":22}],21:[function(require,module,exports){
/**
 * undo、redo 工具包
 */

var ENV = require('./env'),
    CONST = require('./const'),
    utils = require('./utils'),
    domUtils = require('./../domUtils/domExtend'),
    rangeUtils = require('./../rangeUtils/rangeExtend');

var MaxRedo = 100;

var codeMirrorDocList = {

};

var historyUtils = {
    enable: false,
    /**
     * 执行 undo 操作时，触发的回调函数， 返回 history 的缓存集合数量，以及当前 undo、redo 操作游标的所在位置——用于控制 undo、redo 按钮的 disabled
     */
    callback: null,
    /**
     * undo 集合
     */
    stack: [],
    /**
     * undo 集合当前游标位置
     */
    stackIndex: 0,
    /**
     * 初始化 historyUtils 工具包
     */
    init: function () {
        historyUtils.stack = [];
        historyUtils.stackIndex = 0;
    },
    /**
     * 开启 history 功能
     * @param maxRedo
     * @param callback
     */
    start: function (maxRedo, callback) {
        if (maxRedo && maxRedo > 0) {
            MaxRedo = maxRedo;
        }
        historyUtils.enable = true;
        historyUtils.init();
        historyEvent.bind();
        if (callback) {
            historyUtils.callback = callback;
        }
    },
    /**
     * 关闭 history 功能
     */
    stop: function () {
        historyUtils.enable = false;
        historyUtils.init();
        historyEvent.unbind();
    },
    /**
     * 触发 callback
     */
    applyCallback: function () {
        var result;
        if (historyUtils.callback) {
            result = historyUtils.getUndoState();
            historyUtils.callback(result);
        }
        // console.log(JSON.stringify(result));
    },
    // 根据 containerId 获取对应的 CodeMirror Doc 副本
    getCodeMirrorDoc: function(containerId) {
        var cmDoc = codeMirrorDocList[containerId] || null;
        if (cmDoc) {
            cmDoc = cmDoc.copy(true);
        }
        return cmDoc;
    },
    getUndoState: function () {
        return {
            'undoCount': historyUtils.stack.length,
            'undoIndex': historyUtils.stackIndex
        };
    },
    /**
     * undo 操作
     */
    undo: function () {
        // console.log('.....undo....');
        if (!historyUtils.enable ||
            historyUtils.stackIndex <= 0 || historyUtils.stack.length === 0) {
            historyUtils.stackIndex = 0;
            return;
        }
        if (historyUtils.stackIndex >= historyUtils.stack.length) {
            // 专门用于 undo 之前把 editor 最后的状态保存
            historyUtils.saveSnap(true);
        }
        //console.log('.....restoreSnap.....' + historyUtils.stack.length + ',' + historyUtils.stackIndex);
        var historyObj = historyUtils.stack[--historyUtils.stackIndex];
        if (historyObj.nextChange.type === CONST.HISTORY.TYPE.CODE_MIRROR) {
            historyUtils.restoreCodeMirror('undo', historyObj);
        } else {
            // 只有处理 CodeMirror 外的内容时才 save
            saveCmDocList();

            historyUtils.restoreSnap(historyObj.snap);
        }

        ENV.event.call(CONST.EVENT.AFTER_RESTORE_HISTORY, {
            type: historyObj.nextChange.type
        });
        historyUtils.applyCallback();
        if (historyObj.type === CONST.HISTORY.TYPE.COMMON) {
            domUtils.focus();
        }
       // console.log('undo: ' + historyUtils.stackIndex);
    },
    /**
     * redo 操作
     */
    redo: function () {
        // console.log('.....redo....');
        if (!historyUtils.enable ||
            historyUtils.stackIndex >= historyUtils.stack.length - 1) {
            return;
        }

        var historyObj = historyUtils.stack[++historyUtils.stackIndex];
        if (historyObj.type === CONST.HISTORY.TYPE.CODE_MIRROR) {
            historyUtils.restoreCodeMirror('redo', historyObj);
        } else {
            // 只有处理 CodeMirror 外的内容时才 save
            saveCmDocList();
            historyUtils.restoreSnap(historyObj.snap);
        }

        ENV.event.call(CONST.EVENT.AFTER_RESTORE_HISTORY, {
            type: historyObj.type
        });
        historyUtils.applyCallback();
        if (historyObj.type === CONST.HISTORY.TYPE.COMMON) {
            domUtils.focus();
        }
       // console.log('redo: ' + historyUtils.stackIndex);
    },
    /**
     * 保存当前内容的快照
     * @param keepIndex （是否保存快照时不移动游标， 主要用于 undo 操作时保存最后的快照）
     * @param options （用于区分不同模块，目前主要是区分 CodeMirror）
     */
    saveSnap: function (keepIndex, options) {
        if (!historyUtils.enable ||
            (CONST.COMPOSITION_START && (!options || options.type === CONST.HISTORY.TYPE.COMMON))) {
            // CodeMirror 不考虑是否中文输入法输入
            return;
        }

        ENV.event.call(CONST.EVENT.BEFORE_SAVESNAP);

        var nextChange = {
            type: CONST.HISTORY.TYPE.COMMON
        };
        if (options && options.type === CONST.HISTORY.TYPE.CODE_MIRROR) {
            nextChange.type = CONST.HISTORY.TYPE.CODE_MIRROR;
            nextChange.cmContainerId = options.cmContainerId;
        }
        // console.log(nextChange);

        var historyObj = {
            nextChange: nextChange
        };

        var canSave, cm, cmContainerId,
            prevCmContainerId, prevHistoryObj,
            snap, prevSnap;

        prevHistoryObj = historyUtils.stackIndex > 0 ? historyUtils.stack[historyUtils.stackIndex - 1] : null;
        if (prevHistoryObj &&
            prevHistoryObj.nextChange.type == CONST.HISTORY.TYPE.CODE_MIRROR) {
            // CodeMirror 的 History

            // console.log('history  ---- code mirror');

            prevCmContainerId = prevHistoryObj.cmContainerId;
            cmContainerId = prevHistoryObj.nextChange.cmContainerId;
            historyObj.type = CONST.HISTORY.TYPE.CODE_MIRROR;
            historyObj.cmContainerId = cmContainerId;
            cm = getCm(cmContainerId);
            if (cm) {
                // undo - redo 之间 操作时，直接清空 redo
                if (historyUtils.stackIndex >= 0) {
                    historyUtils.stack.splice(historyUtils.stackIndex, historyUtils.stack.length - historyUtils.stackIndex);
                }

                if (historyObj.nextChange.type === CONST.HISTORY.TYPE.COMMON) {
                    // CodeMirror 转到 Common 时需要保存 快照 snap
                    // 否则 undo 时 无法还原 Editor 输入前 的最后一次状态
                    historyObj.snap = historyUtils.snapshot();
                }

                historyObj.cmHistory = cm.historySize();

                // 检查 CodeMirror HistorySize，没有变化则不记录新的 history
                if (prevCmContainerId === cmContainerId &&
                    prevHistoryObj.cmHistory.undo === historyObj.cmHistory.undo &&
                    prevHistoryObj.cmHistory.redo === historyObj.cmHistory.redo
                ) {
                    // 不记录新的 history，但要更新 nextChange
                    prevHistoryObj.nextChange = historyObj.nextChange;
                    if (historyObj.snap) {
                        prevHistoryObj.snap = historyObj.snap;
                    }
                    if (keepIndex) {
                        historyUtils.stackIndex--;
                    }
                    return;
                }

                historyUtils.stack.push(historyObj);
                if (!keepIndex) {
                    historyUtils.stackIndex++;
                }
            }

        } else {
            // 普通正文编辑 的 History
            // console.log('history  ---- html');

            historyObj.type = CONST.HISTORY.TYPE.COMMON;

            canSave = {add: true, replace: false, direct: 0};
            snap = historyUtils.snapshot();
            // keepIndex 为 true 时， canSave.add 肯定为 true
            if (!keepIndex && prevHistoryObj &&
                prevHistoryObj.type == CONST.HISTORY.TYPE.COMMON) {
                canSave = historyUtils.canSave(snap, prevHistoryObj.snap);
            }

            if (canSave.add || canSave.replace) {
                //console.log('save snap.... stack: [' + historyUtils.stack.length + ']  index: [' + historyUtils.stackIndex + ']  keepIndex: [' + !!keepIndex + ']');
                //记录 光标移动方向，用于判断是删除还是添加字符
                snap.direct = canSave.direct;

                // undo - redo 之间 操作时，直接清空 redo
                if (historyUtils.stackIndex >= 0) {
                    historyUtils.stack.splice(historyUtils.stackIndex, historyUtils.stack.length - historyUtils.stackIndex);
                }
               // console.log(snap.content);
                if (canSave.add) {
                    // console.log('save snap.add.... stack: [' + historyUtils.stack.length + ']  index: [' + historyUtils.stackIndex + ']  keepIndex: [' + !!keepIndex + ']');
                    historyObj.snap = snap;
                    historyUtils.stack.push(historyObj);
                    if (!keepIndex) {
                        historyUtils.stackIndex++;
                    }
                } else if (canSave.replace) {
                    //console.log('save snap.replace.... stack: [' + historyUtils.stack.length + ']  index: [' + historyUtils.stackIndex + ']  keepIndex: [' + !!keepIndex + ']');
                    prevSnap = prevHistoryObj.snap;
                    snap.focus.startOffset = prevSnap.focus.startOffset;
                    prevHistoryObj.snap = snap;
                }
            }
        }

        if (historyUtils.stack.length > MaxRedo) {
            historyUtils.stack.shift();
            historyUtils.stackIndex--;
        }

        historyUtils.applyCallback();
        // console.log('stack.length = ' + historyUtils.stack.length + ', index: ' + historyUtils.stackIndex);

        // console.log(historyUtils.stack);
    },
    restoreCodeMirror: function (restoreType, historyObj) {
        if (!historyUtils.enable || !historyObj) {
            return;
        }
        var cmContainerId, cm;
        // console.log('restoreCodeMirror = ' + restoreType);
        // undo 操作 要看下一个改动由谁操作
        // redo 操作 要看当前由谁操作
        cmContainerId = restoreType == 'undo' ? historyObj.nextChange.cmContainerId : historyObj.cmContainerId;
        cm = getCm(cmContainerId);
        if (cm) {
            if (restoreType === 'redo') {
                cm.redo();
            } else {
                cm.undo();
            }
        }

    },
    /**
     * 根据指定的 快照 恢复页面内容
     * @param snap
     */
    restoreSnap: function (snap) {
        if (!historyUtils.enable || !snap) {
            return;
        }

        var sel, start, end;
        sel = ENV.doc.getSelection();
        ENV.doc.body.innerHTML = snap.content;
        try {
            start = domUtils.getFromIndexList(snap.focus.start);
            sel.collapse(start.dom, start.offset);
            if (!snap.focus.isCollapsed) {
                end = domUtils.getFromIndexList(snap.focus.end);
                rangeUtils.setRange(start.dom, start.offset, end.dom, end.offset);
            } else {
                rangeUtils.setRange(start.dom, start.offset, start.dom, start.offset);
            }
            rangeUtils.caretFocus();
        } catch (e) {

        }

    },
    /**
     * 判断 当前快照是否可以保存
     * @param s1
     * @param s2
     * @returns {{add: boolean, replace: boolean}}
     */
    canSave: function (s1, s2) {
        var result = {add: false, replace: false, direct: 0};
        if (s1.content.length != s2.content.length || !!s1.content.localeCompare(s2.content)) {
            result.direct = compareFocus(s1.focus, s2.focus);
            if (result.direct === 0 ||
                result.direct !== s2.direct ||
                (!(/^\s*$/g.test(s1.focus.lastChar + s2.focus.lastChar)) &&
                /\s/.test(s2.focus.lastChar)) ||
                Math.abs(s1.focus.startOffset - s2.focus.startOffset) > 20
            ) {
                // direct 为 0 表明不是在一个 TextNode 操作
                // direct 不相同操作不一致
                // s1 & s2 的 lastChar 不同时为空
                // 连续输入超过 3秒 需要截断
                result.add = true;
            } else {
                result.replace = true;
            }
        }
        // console.log(' ..... can Save .....');
        // console.log((s1.focus.lastChar !== s2.focus.lastChar) + ', ' + (s1.focus.lastChar === ' ') + ', ' + (s2.focus.lastChar === ' '));
        // console.log(s1.direct + ', ' + s2.direct + ', ' + result.direct);
        // console.log(JSON.stringify(s1.focus));
        // console.log(JSON.stringify(s2.focus));
        // console.log(result);
        return result;

        function compareFocus (f1, f2) {
            if (f1.isCollapsed != f2.isCollapsed) {
                return 0;
            }
            if (f1.start.length != f2.start.length ||
                f1.end.length != f2.end.length) {
                return 0;
            }
            var result = compareIndexList(f1.start, f2.start);
            if (result < 1) {
                return result;
            }
            result = compareIndexList(f1.end, f2.end);
            return result;
        }

        function compareIndexList (index1, index2) {
            var isSame = 1,
                i, j;
            for (i = 0, j = index1.length - 1; i < j; i++) {
                if (index1[i] != index2[i]) {
                    isSame = 0;
                    break;
                }
            }
            if (isSame && index1[j] < index2[j]) {
                isSame = -1;
            }
            //console.log('.....compareIndexList.....')
            //console.log(index1)
            //console.log(index2)
            //console.log(isSame)
            return isSame;
        }

    },
    /**
     * 生成快照
     * @returns {{content: string, focus: {isCollapsed: boolean, start: Array, end: Array}}}
     */
    snapshot: function () {
        var sel = ENV.doc.getSelection(),
            content = ENV.doc.body.innerHTML,
            focus = {
                isCollapsed: true,
                start: [],
                end: [],
                lastChar: null,
                startOffset: -1
            },
            snap = {
                content: content,
                focus: focus
            };

        if (sel.rangeCount === 0) {
            focus.start.push(0);
            return snap;
        }

        var range = sel.getRangeAt(0);
        focus.start = domUtils.getIndexList(range.startContainer);
        focus.start.push(range.startOffset);
        focus.isCollapsed = sel.isCollapsed;
        if (!sel.isCollapsed) {
            focus.end = domUtils.getIndexList(range.endContainer);
            focus.end.push(range.endOffset);
        } else if (range.startContainer.nodeType === 3 &&
            range.startOffset > 0) {
            // 判断是否正在输入单词
            focus.lastChar = range.startContainer.nodeValue.charAt(range.startOffset - 1);
            focus.startOffset = range.startOffset;
        }
        return snap;
    }
};

/**
 * 历史记录功能的 事件处理
 */
var historyEvent = {
    /**
     * 初始化时， 绑定历史记录相关的必要事件
     */
    bind: function () {
        historyEvent.unbind();
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, historyEvent.onKeyDown);
    },
    /**
     * 解绑历史记录相关的必要事件
     */
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, historyEvent.onKeyDown);
    },
    /**
     * 快捷键 监控
     * @param e
     */
    onKeyDown: function (e) {

        var keyCode = e.keyCode || e.which;
        //console.log('history keydown.....' + keyCode);

        /**
         * Ctrl + Z
         */
        if ((e.ctrlKey && keyCode == 90) ||
            (e.metaKey && keyCode == 90 && !e.shiftKey)) {
            historyUtils.undo();
            utils.stopEvent(e);
            return;
        }
        /**
         * Ctrl + Y
         */
        if ((e.ctrlKey && keyCode == 89) ||
            (e.metaKey && keyCode == 89) ||
            (e.metaKey && keyCode == 90 && e.shiftKey)) {
            historyUtils.redo();
            utils.stopEvent(e);
        }
    }
};

function getCm (cmContainerId) {
    var cm = null;
    var cmContainer = ENV.doc.querySelector('#' + cmContainerId);
    if (cmContainer && cmContainer.codeMirror) {
        // CodeMirror change 时不用考虑 keepIndex，因为肯定是发生改变了
        cm = cmContainer.codeMirror;
    }
    return cm;
}
function getCmContainerList () {
    return ENV.doc.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER);
}
function saveCmDocList() {
    var containerList = getCmContainerList();
    var container, cm, i;
    for (i=containerList.length - 1; i>=0; i--) {
        container = containerList[i];
        cm = container.codeMirror;
        if (cm) {
            codeMirrorDocList[container.id] = cm.getDoc().copy(true);
        }
    }
    // console.log(codeMirrorDocList);
}

module.exports = historyUtils;
},{"./../domUtils/domExtend":29,"./../rangeUtils/rangeExtend":49,"./const":18,"./env":20,"./utils":24}],22:[function(require,module,exports){
/**
 * Created by ZQG on 2015/3/11.
 */

var LANG = {}, userLangType = 'en', userLang = {};
LANG['en'] = {
    version: 'en',
    Month: ['Jan.', 'Feb.', 'Mar.', 'Apr.', 'May', 'June', 'July', 'Agu.', 'Sep.', 'Oct.', 'Nov.', 'Dec.'],
    Amend: {
        Edit: 'Inserted contents',
        Delete: 'Deleted contents',
        BtnAccept: 'Accept',
        BtnRefuse: 'Reject',
        Accept: 'Accept all changes? Or partially select the changes which need to be accepted.',
        Refuse: 'Reject all changes? Or partially select the changes which need to be rejected.',
        MultiInfo: 'Multiple changes are selected',
        UserNameDefault: 'someone'
    },
    Table: {
        Copy: 'Copy',
        Paste: 'Paste',
        ClearCell: 'Clear',
        MergeCell: 'Merge Cells',
        SplitCell: 'Unmerge Cells',
        InsertRowUp: 'Add Row Above',
        InsertRowDown: 'Add Row Below',
        InsertColLeft: 'Add Column Before',
        InsertColRight: 'Add Column After',
        DeleteRow: 'Delete Row',
        DeleteCol: 'Delete Column',
        SetCellBg: 'Color Fill',
        CellAlign: 'Arrange',
        DeleteTable: 'Delete Table',
        DistributeCols: 'Average Column Width'
    },
    Err: {
        Copy_Null: 'Copy of deleted changes not allowed',
        Cut_Null: 'Cut of deleted changes not allowed'
    }
};
LANG['zh-cn'] = {
    version: 'zh-cn',
    Date: {
        Year: '年',
        Month: '月',
        Day: '日'
    },
    Amend: {
        Edit: '插入了内容',
        Delete: '删除了内容',
        BtnAccept: '接受修订',
        BtnRefuse: '拒绝修订',
        Accept: '是否确认接受全部修订内容？ 如需接受部分内容请使用鼠标进行选择',
        Refuse: '是否确认拒绝全部修订内容？ 如需拒绝部分内容请使用鼠标进行选择',
        MultiInfo: '您选中了多处修订',
        UserNameDefault: '有人'
    },
    Table: {
        Copy: '复制',
        Paste: '粘贴',
        ClearCell: '清空单元格',
        MergeCell: '合并单元格',
        SplitCell: '拆分单元格',
        InsertRowUp: '上插入行',
        InsertRowDown: '下插入行',
        InsertColLeft: '左插入列',
        InsertColRight: '右插入列',
        DeleteRow: '删除当前行',
        DeleteCol: '删除当前列',
        SetCellBg: '单元格底色',
        CellAlign: '单元格对齐方式',
        DeleteTable: '删除表格',
        DistributeCols: '平均分配各列'
    },
    Err: {
        Copy_Null: '无法复制已删除的内容',
        Cut_Null: '无法剪切已删除的内容'
    }
};
LANG['zh-tw'] = {
    version: 'zh-tw',
    Date: {
        Year: '年',
        Month: '月',
        Day: '日'
    },
    Amend: {
        Edit: '插入了內容',
        Delete: '刪除了內容',
        BtnAccept: '接受修訂',
        BtnRefuse: '拒絕修訂',
        Accept: '是否確認接受全部修訂內容？ 如需接受部分內容請使用滑鼠進行選擇',
        Refuse: '是否確認拒絕全部修訂內容？ 如需拒絕部分內容請使用滑鼠進行選擇',
        MultiInfo: '您選中了多處修訂',
        UserNameDefault: '有人'
    },
    Table: {
        Copy: '複製',
        Paste: '粘貼',
        ClearCell: '清空儲存格',
        MergeCell: '合併儲存格',
        SplitCell: '拆分儲存格',
        InsertRowUp: '上插入行',
        InsertRowDown: '下插入行',
        InsertColLeft: '左插入列',
        InsertColRight: '右插入列',
        DeleteRow: '刪除當前行',
        DeleteCol: '刪除當前列',
        SetCellBg: '儲存格底色',
        CellAlign: '儲存格對齊方式',
        DeleteTable: '刪除表格',
        DistributeCols: '平均分配各列'
    },
    Err: {
        Copy_Null: '無法複製已刪除的內容',
        Cut_Null: '無法剪切已刪除的內容'
    }
};

function setLang(type) {
    if (!type) {
        type = 'en';
    }
    //同时支持 zh-cn & zh_cn
    type = type.toLowerCase().replace('_', '-');
    if (LANG[type]) {
        userLangType = type;
    } else {
        type = 'en';
    }

    var k;
    for (k in LANG[type]) {
        if (LANG[type].hasOwnProperty(k)) {
            userLang[k] = LANG[type][k];
        }
    }
}

module.exports.getLang = function () {
    return userLang;
};
module.exports.initLang = function (type) {
    setLang(type);
};

},{}],23:[function(require,module,exports){
/*
 *用于加载js
 *options是数组，值有
 *  字符串：js地址
 *  对象(js需保存到localStorage)：
 *      {
 *         id:"",
 *         version:"",
 *         link:""
 *      }
 */

var scriptLoader = {
    appendJsCode: function (doc, jsStr, type) {
        var s = doc.createElement('script');
        s.type = type;
        s.text = jsStr;
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].appendChild(s);
    },
    load: function (doc, options, callback) {
        if (!doc || !options) {
            return;
        }
        var i, j, s, c, id = (new Date()).valueOf(), allLoaded = true;
        for (i = 0, j = options.length; i < j; i++) {
            if (typeof (options[i]) == "string") {
                s = this.loadSingleJs(doc, options[i]);
                if (s !== true) {
                    s.onload = makeLoadHandle(id, callback);
                    allLoaded = false;
                }
            }
            else {
                var jsUrl = options[i].link, jsId = createJsId(options[i].id), jsVersion = options[i].version;
                if (window.localStorage) {
                    var jsInfo = JSON.parse(localStorage.getItem(jsId));
                    if (jsInfo && jsInfo.version == jsVersion) {
                        s = this.inject(doc, jsInfo.jsStr, jsId);
                        if (s !== true) {
                            c = makeLoadHandle(id, callback);
                            setTimeout(function () {
                                c();
                            }, 10);
                            allLoaded = false;
                        }
                    }
                    else {
                        allLoaded = false;
                        c = makeLoadHandle(id, callback);
                        $.ajax({
                            url: jsUrl,
                            context: {id: jsId, version: jsVersion},
                            success: function (data) {
                                save({id: this.id, version: this.version, jsStr: data});
                                s = wizUI.scriptLoader.inject(doc, data, this.id);
                                if (s !== true) {
                                    setTimeout(function () {
                                        c();
                                    }, 10);
                                }
                            },
                            error: function () {
                                c();
                            }
                        });
                    }
                }
                else {
                    s = this.loadSingleJs(doc, options[i].link);
                    if (s !== true) {
                        s.onload = makeLoadHandle(id, callback);
                        allLoaded = false;
                    }
                }
            }

        }
        if (allLoaded) {
            callback();
        }
    },
    loadSingleJs: function (doc, path) {
        var jsId = 'wiz_' + path;
        if (doc.getElementById(jsId)) {
            return true;
        }
        var s = doc.createElement('script');
        s.type = 'text/javascript';
        s.setAttribute('charset', "utf-8");
        s.src = path.replace(/\\/g, '/');
        s.id = jsId;
        //s.className = utils.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    },
    inject: function (doc, jsStr, jsId) {
        if (!doc || doc.getElementById(jsId)) {
            return true;
        }
        var s = doc.createElement("script");
        s.type = 'text/javascript';
        s.id = jsId;
        s.text = jsStr;
        //s.className = utils.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    }
};
var loadCount = {};

function makeLoadHandle(id, loadCallback) {
    if (!loadCount[id]) {
        loadCount[id] = 0;
    }
    loadCount[id]++;
    return function () {
        loadCount[id]--;
        if (loadCount[id] === 0) {
            loadCount[id] = null;
            if (loadCallback) {
                loadCallback();
            }
        }
    }
}

function createJsId(jsId) {
    return "wiz_js_" + jsId;
}

function save(options) {
    if (!options) {
        return;
    }
    var jsInfo = {
        version: options.version,
        jsStr: options.jsStr
    };
    localStorage.setItem(options.id, JSON.stringify(jsInfo));
}

module.exports = scriptLoader;
},{}],24:[function(require,module,exports){
var CONST = require('./const');

// 填坑：安卓版 微信浏览器（6.5.4） localeCompare 错误 'Uncaught illegal access'
// MQQBrowser/6.2 TBS/043024 Safari/537.36 MicroMessenger/6.5.4.1000
try {
    'abc'.localeCompare('abcd');
} catch (e) {
    String.prototype.localeCompare = function (str) {
        return this.toString() == str ? 0 : (this.toString() > str ? 1 : -1);
    };
}

// package trim()
var rtrim = /^[\s\uFEFF\xA0]+|[\s\uFEFF\xA0]+$/g;
if (!"wiz".trim) {
    String.prototype.trim = function () {
        return this.replace(rtrim, "");
    }
}

// package regexp 特殊字符替换
var matchRegExpKeyWord = /[\-\[\]{}()*+?.,\\\^$|#\s]/g;
try {
    'abc'.escapeRegex('abcd');
} catch (e) {
    String.prototype.escapeRegex = function () {
        return this.toString().replace(matchRegExpKeyWord, '\\$&');
    };
}

// package indexOf()
if (!Array.prototype.indexOf) {
    Array.prototype.indexOf = function (n) {
        for (var i = 0; i < this.length; i++) {
            if (this[i] == n) {
                return i;
            }
        }
        return -1;
    }
}

/**
 * 常用基本工具包
 */
var utils = {
    /**
     * 检查 是否为有效的键盘输入内容
     * @param e
     * @returns {boolean}
     */
    checkNonTxtKey: function (e) {
        var keyCode = e.keyCode || e.which;
        if (e.ctrlKey || e.metaKey) {
            return true;
        }
        return !( (keyCode >= 48 && keyCode <= 57) || //0-9
            (keyCode >= 65 && keyCode <= 90) || //a-z
            (keyCode >= 96 && keyCode <= 107) || //小键盘0-9 * +
            (keyCode >= 109 && keyCode <= 111) || //小键盘 / * -
            (keyCode >= 186 && keyCode <= 192) || //标点符号
            (keyCode >= 219 && keyCode <= 222) || //标点符号
            keyCode == 229 || keyCode === 0 || //中文
            keyCode == 13 || //Enter
            keyCode == 32  //空格
        );
    },
    /**
     * 判断 obj 是否为 数组
     * @param obj
     * @returns {boolean}
     */
    isArray: function (obj) {
        return Object.prototype.toString.apply(obj) === "[object Array]";
    },
    /**
     * 判断字符串是否为空， 空格 不认为空
     * @param str
     * @returns {boolean}
     */
    isEmpty: function (str) {
        if (!str) {
            return true;
        }
        var enter = /\r?\n/ig,
            r = new RegExp('[\r\n' + CONST.FILL_CHAR + ']', 'ig'),
            hasEnter = enter.test(str),
            _str = str.replace(r, ''),
            isNone = str.replace(r, '').trim().length === 0;
        //避免 正常标签只存在 一个空格时，也被误判
        return _str.length === 0 || (hasEnter && isNone);
    },
    /**
     * 判断两个 修订时间是否近似相同
     * @param time1
     * @param time2
     * @returns {boolean}
     */
    isSameAmendTime: function (time1, time2) {
        if (!time1 || !time2) {
            return false;
        }
        var t1 = utils.getDateForTimeStr(time1),
            t2 = utils.getDateForTimeStr(time2);
        return Math.abs(t1 - t2) <= CONST.AMEND_BATCH_TIME_SPACE;
    },
    checkChar: function (char, rangeList) {
        var i, j, start, end;
        for (i = 0, j = rangeList.length; i < j; i++) {
            start = rangeList[i][0];
            end = rangeList[i].length > 1 ? rangeList[i][1] : null;
            if (end === null && char == start) {
                return true;
            } else if (end !== null && char >= start && char <= end) {
                return true;
            }
        }
        return false;
    },
    compareVersion: function (v1, v2) {
        v1 = v1.split('.');
        v2 = v2.split('.');

        var i, j, n1, n2;
        for (i=0, j=v1.length; i<j; i++) {
            n1 = parseInt(v1[i], 10);
            n2 = v2[i] ? parseInt(v2[i], 10) : -1;

            if (n1 < n2) {
                return -1;
            } else if (n1 > n2) {
                return 1;
            }
        }

        if (v1.length < v2.length) {
            return -1;
        }
        return 0;
    },
    isCJK: function (char) {
        var CJKRange = [
            [0x3040, 0x318f], [0x3300, 0x337f], [0x3400, 0x3d2d],
            [0x4e00, 0x9fff], [0xf900, 0xfaff], [0xac00, 0xd7af]
        ];
        return utils.checkChar(char, CJKRange);
    },
    isSpace: function (char) {
        var SpaceRange = [
            [0x0009, 0x000D], [0x0020], [0x00A0]
        ];
        return utils.checkChar(char, SpaceRange);
    },
    isAlpha: function (char) {
        var AlphaRange = [
            [0x0030, 0x0039], [0x0041, 0x005A], [0x0061, 0x007A],
            [0x00C0, 0x00D6], [0x00D8, 0x00F6], [0x00F8, 0x100]
        ];
        return utils.checkChar(char, AlphaRange);
    },
    getWordCount: function (str) {
        var count = {
            nWords: 0,
            nChars: 0,
            nCharsWithSpace: 0,
            nNonAsianWords: 0,
            nAsianChars: 0
        };
        if (!str) {
            return count;
        }
        var i, j, ch;
        var isAlpha = false;
        var isInWords = false;
        count.nCharsWithSpace = str.length;  //字符串长度
        count.nChars = count.nCharsWithSpace;
        for (i = 0, j = str.length; i < j; i++) {
            ch = str.charCodeAt(i);
            isAlpha = false;
            if (utils.isCJK(ch)) {
                // 中文
                count.nAsianChars++;
            } else if (utils.isSpace(ch)) {
                // 空格
                count.nChars--;
            } else if (utils.isAlpha(ch)) {
                //字母
                isAlpha = true;
            }
            // console.log(ch);
            // console.log('isAlpha: ' + isAlpha + ', isInWords: ' + isInWords );

            if (isAlpha && !isInWords) {
                isInWords = true;
            } else if (!isAlpha) {
                if (isInWords) {
                    count.nNonAsianWords++;
                }
                isInWords = false;
            }
        }
        // 修正最后一个单词
        if (isAlpha && isInWords) {
            count.nNonAsianWords++;
        }
        count.nWords = count.nNonAsianWords + count.nAsianChars;
        return count;
    },
    getEventClientPos: function (e) {
        return {
            x: e.changedTouches ? e.changedTouches[0].clientX : e.clientX,
            y: e.changedTouches ? e.changedTouches[0].clientY : e.clientY
        };
    },
    /**
     * 获取字符串 的 hash 值
     * @param str
     * @returns {number}
     */
    getHash: function (str) {
        var hash = 1315423911, i, ch;
        for (i = str.length - 1; i >= 0; i--) {
            ch = str.charCodeAt(i);
            hash ^= ((hash << 5) + ch + (hash >> 2));
        }
        return (hash & 0x7FFFFFFF);
    },
    /**
     * 生成当前时间戳，用于 修订的时间
     * @returns {string}
     */
    getTime: function () {
        var d = new Date();
        return d.getFullYear() + '-' + to2(d.getMonth() + 1) + '-' + to2(d.getDate()) +
            ' ' + to2(d.getHours()) + ':' + to2(d.getMinutes()) + ':' + to2(d.getSeconds());

        function to2 (num) {
            var str = num.toString();
            return str.length == 1 ? '0' + str : str;
        }
    },
    /**
     * 根据 日期字符串 返回 Date 对象（用于修订编辑，所以只支持 yyyy-mm-hh HH:MM:SS 格式）
     * @param str
     * @returns {Date}
     */
    getDateForTimeStr: function (str) {
        return new Date(Date.parse(str.replace(/-/g, "/")));
    },
    /**
     * 将 list 转换为 Map （主要用于处理 tagNames）
     * @param list
     * @returns {{}}
     */
    listToMap: function (list) {
        if (!list) {
            return {};
        }
        list = utils.isArray(list) ? list : list.split(',');
        var i, j, ci, obj = {};
        for (i = 0, j = list.length; i < j; i++) {
            ci = list[i];
            obj[ci.toUpperCase()] = obj[ci] = 1;
        }
        return obj;
    },
    rgb2Hex: function (str) {
        if (!str) {
            return '';
        }
        var rgb = str.replace(/.*\((.*)\)/ig, '$1').split(',');
        if (rgb.length < 3) {
            return '';
        }
        var r = parseInt(rgb[0], 10),
            g = parseInt(rgb[1], 10),
            b = parseInt(rgb[2], 10),
            a = rgb.length === 4 ? parseFloat(rgb[3]) : 1;
        if (a === 0) {
            return '';
        }
        return '#' + getHex(getColor(r, a)) + getHex(getColor(g, a)) + getHex(getColor(b, a));

        function getColor (color, colorA) {
            return color + Math.floor((255 - color) * (1 - colorA));
        }

        function getHex (n) {
            var h = n.toString(16);
            return h.length === 1 ? '0' + h : h;
        }
    },
    /**
     * 删除 数组中重复的数据
     * @param arr
     * @returns {Array}
     */
    removeDup: function (arr) {
        var result = [], i, j, a;
        for (i = 0, j = arr.length; i < j; i++) {
            a = arr[i];
            if (result.indexOf(a) < 0) {
                result.push(a);
            }
        }
        return result;
    },
    replaceSpecialChar: function(text) {
        // 替换 C2A0 为普通空格
        // 删除 占位符
        text = text.replace(new RegExp(String.fromCharCode(160), 'g'), ' ')
            .replace(String.fromCharCode(65279), '');
        return text;
    },
    /**
     * 阻止默认事件
     * @param e
     */
    stopEvent: function (e) {
        if (!e) {
            return;
        }
        e.stopPropagation();
        e.preventDefault();
        //这个会阻止其他同event 的触发，过于野蛮
        //e.stopImmediatePropagation();
    },
    //-------------------- 以下内容修改需要 保证与 wizUI 中的 utils 内 对应方法一致 start ----------------------
    //PcCustomTagClass: 'wiz-html-render-unsave', //此 class 专门用于 pc 端将 markdown 笔记选然后发email 或微博等处理
    WizEditorTmpName: 'wiz_tmp_editor_style', //WizEditor 专用临时 style
    loadSingleCss: function (doc, path) {
        var cssId = 'wiz_' + path;
        if (doc.getElementById(cssId)) {
            return true;
        }

        var s = doc.createElement('link');
        s.rel = 'stylesheet';
        s.setAttribute('charset', "utf-8");
        s.setAttribute('name', this.WizEditorTmpName);
        s.href = path.replace(/\\/g, '/');
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].insertBefore(s, null);
        return s;
    },
    appendCssCode: function (doc, jsStr, type) {
        var s = doc.createElement('style');
        s.type = type;
        s.text = jsStr;
        s.setAttribute('name', this.WizEditorTmpName);
        //s.className = this.PcCustomTagClass;
        doc.getElementsByTagName('head')[0].appendChild(s);
    },
    /**
     * FF下无法获取innerText，通过解析DOM树来解析innerText，来渲染markdown
     * @param ele 需要解析的节点元素
     * @returns {string}
     */
    getInnerText: function (ele) {

        var t = '';

        var normalize = function (a) {
            if (!a) {
                return "";
            }
            return a.replace(/ +/gm, " ")
                .replace(/[\t]+/gm, "")
                .replace(/[ ]+$/gm, "")
                .replace(/^[ ]+/gm, "")
                .replace(/\n+/gm, "\n")
                .replace(/\n+$/, "")
                .replace(/^\n+/, "")
                .replace(/NEWLINE/gm, '\n')
            //return a.replace(/ +/g, " ")
            //    .replace(/[\t]+/gm, "")
            //    .replace(/[ ]+$/gm, "")
            //    .replace(/^[ ]+/gm, "")
            //    .replace(/\n+/g, "\n")
            //    .replace(/\n+$/, "")
            //    .replace(/^\n+/, "")
        };
        var removeWhiteSpace = function (node) {
            // 去掉空的文本节点
            var isWhite = function (node) {
                return !(/[^\t\n\r ]/.test(node.nodeValue));
            };
            var ws = [];
            var findWhite = function (node) {
                for (var i = 0; i < node.childNodes.length; i++) {
                    var n = node.childNodes[i];
                    if (n.nodeType == 3 && isWhite(n)) {
                        ws.push(n);
                    } else if (n.hasChildNodes()) {
                        findWhite(n);
                    }
                }
            };
            findWhite(node);
            for (var i = 0; i < ws.length; i++) {
                ws[i].parentNode.removeChild(ws[i]);
            }
        };
        var sty = function (n, prop) {
            // 获取节点的style
            if (n.style[prop]) {
                return n.style[prop];
            }
            var s = n.currentStyle || n.ownerDocument.defaultView.getComputedStyle(n, null);
            if (n.tagName == "SCRIPT") {
                return "none";
            }
            if (!s[prop]) {
                return "LI,P,TR".indexOf(n.tagName) > -1 ? "block" : n.style[prop];
            }
            if (s[prop] == "block" && n.tagName == "TD") {
                return "feaux-inline";
            }
            return s[prop];
        };

        var blockTypeNodes = "table-row,block,list-item";
        var isBlock = function (n) {
            // 判断是否为block元素
            var s = sty(n, "display") || "feaux-inline";
            return blockTypeNodes.indexOf(s) > -1;

        };
        // 遍历所有子节点，收集文本内容，注意需要空格和换行
        var recurse = function (n) {
            // 处理pre元素
            if (/pre/.test(sty(n, "whiteSpace"))) {
                t += n.innerHTML
                    .replace(/\t/g, " ")
                return "";
            }
            var s = sty(n, "display");
            if (s == "none") {
                return "";
            }
            var gap = isBlock(n) ? "\n" : " ";
            t += gap;
            for (var i = 0; i < n.childNodes.length; i++) {
                var c = n.childNodes[i];
                if (c.nodeType == 3) {
                    t += c.nodeValue;
                }

                if (c.childNodes.length) {
                    recurse(c);
                }
            }
            t += gap;
            return t;
        };

        var node = ele.cloneNode(true);
        // br转换成会忽略换行, 会出现 <span>aaa</span><br><span>bbb</span> 的情况，因此用一个特殊字符代替，而不是直接替换成 \n
        node.innerHTML = node.innerHTML.replace(/<br[\/]?>/gi, 'NEWLINE');

        // p元素会多一个换行，暂时用NEWLINE进行占位，markdown中不考虑p元素
        //var paras = node.getElementsByTagName('p');
        //for(var i = 0; i < paras.length; i++) {
        //    paras[i].innerHTML += 'NEWLINE';
        //}
        removeWhiteSpace(node);
        return normalize(recurse(node));
    },

    /**
     * 对markdown的html内容进行预处理，已显示图片，todoList等等
     * @param dom 传入的dom对象
     */
    markdownPreProcess: function (dom) {
        var el = $(dom);
        //处理 table 容器
        el.find('.wiz-table-container').each(function (index) {
            var target = $(this);
            var span = $("<span></span>");
            span.text(target[0].outerHTML);
            span.insertAfter(target);
            var br = $('<br/>');
            br.insertAfter(target);
            target.remove();
        });
        //处理 旧版本 todolist
        el.find('label.wiz-todo-label').each(function (index) {
            var target = $(this);
            //检测如果是遗留的 label 则不进行特殊处理
            var img = $('.wiz-todo-img', this);
            if (img.length === 0) {
                return;
            }

            var span = $("<span></span>");
            //避免 父节点是 body 时导致笔记阅读异常
            span.text(target[0].outerHTML);
            span.insertAfter(target);
            target.remove();
        });
        //处理 todolist
        el.find('.wiz-todo-layer').each(function (index) {
            var target = $(this);
            //检测如果是遗留的 todoList 则不进行特殊处理
            var checkbox = $('.wiz-todo-checkbox', this);
            if (checkbox.length === 0) {
                return;
            }

            var span = $("<span></span>");
            //避免 父节点是 body 时导致笔记阅读异常
            span.text(target[0].outerHTML);
            span.insertAfter(target);
            target.remove();
        });
        //处理 a 超链接
        el.find('a').each(function (index, link) {
            var linkObj = $(link);
            var href = linkObj.attr('href');
            if (href && /^(wiz|wiznote):/.test(href)) {
                var span = $("<span></span>");
                span.text(linkObj[0].outerHTML);
                span.insertAfter(linkObj);
                linkObj.remove();
            }
        });
        // 必须先处理 A 标签，后处理 Img，否则 A 标签包含 Img 后 会导致 Img 被转义为 html 源码
        //处理 img
        el.find('img').each(function (index) {
            var target = $(this);
            var span = $("<span></span>");
            span.text(target[0].outerHTML);
            span.insertAfter(target);
            target.remove();
        });

        //处理段落 p
        el.find('p').each(function () {
            var target = $(this);
            target.replaceWith($('<div>' + this.innerHTML + '</div>'));
        });
    }
    //-------------------- 以上内容修改需要 保证与 wizUI 中的 utils 内 对应方法一致 end ----------------------
};

module.exports = utils;
},{"./const":18}],25:[function(require,module,exports){
/**
 * 默认的样式集合
 */
var ENV = require('./env'),
    CONST = require('./const'),
    domUtils = require('../domUtils/domBase');

var TmpEditorStyle = {
        iosPhone: 'body {' +
        'overflow-y:scroll;' +
        '-webkit-overflow-scrolling: touch;' +
        '-webkit-tap-highlight-color: rgba(0, 0, 0, 0);' +
        '}' +
        'body[contenteditable=true], .wiz-template-editable {' +
        'padding-bottom: 44px !important;' +
        '}' +
        'td,th {position:static;}' +
        'th:before,td:before,th:after,td:after {display:none;}',
        iosPad: 'body {' +
        'min-width: 90%;' +
        'max-width: 100%;' +
        'min-height: 100%;' +
        'background: #ffffff;' +
        'overflow-y:scroll;' +
        '-webkit-overflow-scrolling: touch;' +
        '-webkit-tap-highlight-color: rgba(0, 0, 0, 0);' +
        '}' +
        'body[contenteditable=true], .wiz-template-editable {' +
        'padding-bottom: 44px !important;' +
        '}' +
        'td,th {position:static;}' +
        'th:before,td:before,th:after,td:after {display:none;}',
        imageResize: '.wiz-img-resize-handle {position: absolute;z-index: 1000;border: 1px solid black;background-color: white;}' +
        '.wiz-img-resize-handle {width:5px;height:5px;}' +
        '.wiz-img-resize-handle.lt {cursor: nw-resize;}' +
        '.wiz-img-resize-handle.tm {cursor: n-resize;}' +
        '.wiz-img-resize-handle.rt {cursor: ne-resize;}' +
        '.wiz-img-resize-handle.lm {cursor: w-resize;}' +
        '.wiz-img-resize-handle.rm {cursor: e-resize;}' +
        '.wiz-img-resize-handle.lb {cursor: sw-resize;}' +
        '.wiz-img-resize-handle.bm {cursor: s-resize;}' +
        '.wiz-img-resize-handle.rb {cursor: se-resize;}',
        selectPlugin: '.wiz-select-plugin-container {position:relative;display:inline-block;width:160px;height:28px;border-radius:4px;padding:0;margin-left:5px;cursor:pointer;}' +
        '.wiz-select-plugin-container, .wiz-select-plugin-options {box-sizing:border-box;background:white;border:1px solid #e7e7e7;color:#333;box-shadow: 1px 1px 5px #d0d0d0;}' +
        '.wiz-select-plugin-options {display:none;}' +
        '.wiz-select-plugin-header {line-height:28px;font-size:14px;padding: 0 0 0 5px;overflow:hidden;margin-right:27px;white-space:nowrap;}' +
        '.wiz-select-plugin-header i {position:absolute;top:0;right:0;line-height:26px;padding:0 6px;border-left:1px solid #e7e7e7;box-sizing:border-box;}' +
        '.wiz-select-plugin-options {background:white;list-style:none;padding:0px;white-space:nowrap;max-height:200px;min-width:160px;max-width:260px;overflow-x:hidden;overflow-y:auto;position:absolute;top:30px;right:-1px;}' +
        '.wiz-select-plugin-options:hover .wiz-select-plugin-options-item.selected {background:white;color:inherit;}' +
        '.wiz-select-plugin-options-item {font-size:11px;padding:0 5px;height:20px;line-height:20px;}' +
        '.wiz-select-plugin-options-item.selected {background:#448aff;color:white;}' +
        '.wiz-select-plugin-options-item:hover {background:#448aff !important;color:white !important;}' +
        '.wiz-select-plugin-container.active .wiz-select-plugin-options {display:block;}',
        table: '.' + CONST.CLASS.TABLE_BODY + '.' + CONST.CLASS.TABLE_MOVING + ' *,' +
        ' .' + CONST.CLASS.TABLE_BODY + '.' + CONST.CLASS.TABLE_MOVING + ' *:before,' +
        ' .' + CONST.CLASS.TABLE_BODY + '.' + CONST.CLASS.TABLE_MOVING + ' *:after {cursor:default !important;}' +
        'td,th {position:relative;}' +
        '#wiz-table-range-border {display: none;width: 0;height: 0;position: absolute;top: 0;left: 0; z-index:' + CONST.CSS.Z_INDEX.tableBorder + '}' +
        '#wiz-table-col-line, #wiz-table-row-line {display: none;background-color: #448aff;position: absolute;z-index:' + CONST.CSS.Z_INDEX.tableColRowLine + ';}' +
        '#wiz-table-col-line {width: 1px;cursor:col-resize;}' +
        '#wiz-table-row-line {height: 1px;cursor:row-resize;}' +
        '#wiz-table-range-border_start, #wiz-table-range-border_range {display: none;width: 0;height: 0;position: absolute;}' +
        '#wiz-table-range-border_start_top, #wiz-table-range-border_range_top {height: 2px;background-color: #448aff;position: absolute;top: 0;left: 0;}' +
        '#wiz-table-range-border_range_top {height: 1px;}' +
        '#wiz-table-range-border_start_right, #wiz-table-range-border_range_right {width: 2px;background-color: #448aff;position: absolute;top: 0;}' +
        '#wiz-table-range-border_range_right {width: 1px;}' +
        '#wiz-table-range-border_start_bottom, #wiz-table-range-border_range_bottom {height: 2px;background-color: #448aff;position: absolute;top: 0;}' +
        '#wiz-table-range-border_range_bottom {height: 1px;}' +
        '#wiz-table-range-border_start_left, #wiz-table-range-border_range_left {width: 2px;background-color: #448aff;position: absolute;top: 0;left: 0;}' +
        '#wiz-table-range-border_range_left {width: 1px;}' +
        '#wiz-table-range-border_start_dot, #wiz-table-range-border_range_dot {width: 5px;height: 5px;border: 2px solid rgb(255, 255, 255);background-color: #448aff;cursor: crosshair;position: absolute;z-index:' + CONST.CSS.Z_INDEX.tableRangeDot + ';}' +
        '.wiz-table-tools {display: block;background-color:#fff;position: absolute;left: 0px;border: 1px solid #ddd;-webkit-border-radius: 5px;-moz-border-radius: 5px;border-radius: 5px;z-index:' + CONST.CSS.Z_INDEX.tableTools + ';}' +
        '.wiz-table-tools ul {list-style: none;padding: 0;width:auto;}' +
        '.wiz-table-tools .wiz-table-menu-item {position: relative;float: left;margin:5px 2px 5px 8px;clear: initial;}' +
        '.wiz-table-tools .wiz-table-menu-item .wiz-table-menu-button {font-size:15px;width:20px;height:20px;line-height:20px;cursor: pointer;position:relative;}' +
        '.wiz-table-tools i.editor-icon{font-size: 15px;color: #455a64;}' +
        '.wiz-table-tools .wiz-table-menu-item .wiz-table-menu-button i#wiz-menu-bg-demo{position: absolute;top:1px;left:0;}' +
        '.wiz-table-tools .wiz-table-menu-sub {position: absolute;display: none;width: 125px;padding: 5px 0;background: #fff;border-radius: 3px;border: 1px solid #E0E0E0;top:28px;left:-9px;box-shadow: 1px 1px 5px #d0d0d0;}' +
        '.wiz-table-tools .wiz-table-menu-sub > div{font-size:15px;}' +
        '.wiz-table-tools .wiz-table-menu-item.active .wiz-table-menu-sub {display: block}' +
        '.wiz-table-tools .wiz-table-menu-sub:before, .wiz-table-tools .wiz-table-menu-sub:after {position: absolute;content: " ";border-style: solid;border-color: transparent;border-bottom-color: #cccccc;left: 22px;margin-left: -14px;top: -8px;border-width: 0 8px 8px 8px;z-index:' + CONST.CSS.Z_INDEX.tableToolsArrow + ';}' +
        '.wiz-table-tools .wiz-table-menu-sub:after {border-bottom-color: #ffffff;top: -7px;}' +
        '.wiz-table-tools .wiz-table-menu-sub-item {padding: 4px 12px;font-size: 14px;}' +
        '.wiz-table-tools .wiz-table-menu-sub-item.split {border-top: 1px solid #E0E0E0;}' +
        '.wiz-table-tools .wiz-table-menu-sub-item:hover {background-color: #ececec;}' +
        '.wiz-table-tools .wiz-table-menu-sub-item.disabled {color: #bbbbbb;cursor: default;}' +
        '.wiz-table-tools .wiz-table-menu-sub-item.disabled:hover {background-color: transparent;}' +
        '.wiz-table-tools .wiz-table-menu-item.wiz-table-cell-bg:hover .wiz-table-color-pad {display: block;}' +
        '.wiz-table-tools .wiz-table-color-pad {display: none;padding: 10px;box-sizing: border-box;width: 85px;height: 88px;background-color: #fff;cursor: default;}' +
        '.wiz-table-tools .wiz-table-color-pad > div{font-size:15px;}' +
        '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item {display: inline-block;width: 15px;height: 15px;margin-right: 9px;position: relative;}' +
        '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item i.pad-demo {position: absolute;top:3px;left:0;}' +
        '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item .icon-oblique_line{color: #cc0000;}' +
        '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item:last-child {margin-right: 0;}' +
        '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item.active i.editor-icon.icon-box {color: #448aff;}' +
        '.wiz-table-tools .wiz-table-cell-align {display: none;padding: 10px;box-sizing: border-box;width: 85px;height: 65px;background-color: #fff;cursor: default;}' +
        '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item {display: inline-block;width: 15px;height: 15px;margin-right: 9px;position: relative;}' +
        '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item:last-child {margin-right:0}' +
        '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item i.valign{position: absolute;top:3px;left:0;color: #d2d2d2;}' +
        '.wiz-table-tools .wiz-table-cell-align-item.active i.editor-icon.valign {color: #a1c4ff;}' +
        '.wiz-table-tools .wiz-table-cell-align-item.active i.editor-icon.icon-box,' +
        '.wiz-table-tools .wiz-table-cell-align-item.active i.editor-icon.align {color: #448aff;}' +
        '.wiz-table-tools .wiz-table-color-pad .wiz-table-color-pad-item:last-child,' +
        '.wiz-table-tools .wiz-table-cell-align .wiz-table-cell-align-item:last-child {margin-right: 0;}' +
        'th.wiz-selected-cell-multi, td.wiz-selected-cell-multi {background: rgba(0,102,255,.05);}' +
        'th:before,td:before,#wiz-table-col-line:before,#wiz-table-range-border_start_right:before,#wiz-table-range-border_range_right:before{content: " ";position: absolute;top: 0;bottom: 0;right: -5px;width: 9px;cursor: col-resize;background: transparent;z-index:' + CONST.CSS.Z_INDEX.tableTDBefore + ';}' +
        'th:after,td:after,#wiz-table-row-line:before,#wiz-table-range-border_start_bottom:before,#wiz-table-range-border_range_bottom:before{content: " ";position: absolute;left: 0;right: 0;bottom: -5px;height: 9px;cursor: row-resize;background: transparent;z-index:' + CONST.CSS.Z_INDEX.tableTDBefore + ';}'
    },
    TmpReaderStyle = {
        ios: 'img {' +
        'max-width: 100%;' +
        'height: auto !important;' +
        'margin: 0px auto;' +
        'cursor: pointer;' + //专门用于 ios 点击 img 触发 click 事件
        '}'
    },
    TmpCommonStyle = {
        body: 'html {height:100%;} body {min-height:100%;box-sizing:border-box;word-wrap: break-word !important;}' +
        'a {word-wrap: break-word;}' +
        'img::selection {background-color: rgba(0, 0, 255, 0.3);}',
        blockScroll: '.wiz-block-scroll::-webkit-scrollbar {width: 7px;height: 7px;}' +
        '.wiz-block-scroll::-webkit-scrollbar-thumb {background-color: #7f7f7f;border-radius: 7px;}' +
        '.wiz-block-scroll::-webkit-scrollbar-button {display: none;}',
        table: '.' + CONST.CLASS.TABLE_CONTAINER + ' {border:0px !important;}' +
        '.' + CONST.CLASS.TABLE_BODY + ' {border:0px !important;position:relative;margin:10px 0;overflow-x:auto;overflow-y:hidden;-webkit-overflow-scrolling:touch;}' +
        '.' + CONST.CLASS.TABLE_BODY + ' table {margin:0;outline:none;}' +
        'td,th {outline:none;}'
    },
    DefaultFont = 'Helvetica, "Hiragino Sans GB", "微软雅黑", "Microsoft YaHei UI", SimSun, SimHei, arial, sans-serif;',
    DefaultStyle = {
        common: 'html, body {' +
        'font-size: 12pt;' +
        '}' +
        'body {' +
        'font-family: ' + DefaultFont +
        'line-height: 1.6;' +
        'margin: 0 auto;' +
        'padding: 20px 16px;padding: 1.25rem 1rem;' +
        '}' +
        'h1, h2, h3, h4, h5, h6 {margin:20px 0 10px;margin:1.25rem 0 0.625rem;padding: 0;font-weight: bold;}' +
        'h1 {font-size:20pt;font-size:1.67rem;}' +
        'h2 {font-size:18pt;font-size:1.5rem;}' +
        'h3 {font-size:15pt;font-size:1.25rem;}' +
        'h4 {font-size:14pt;font-size:1.17rem;}' +
        'h5 {font-size:12pt;font-size:1rem;}' +
        'h6 {font-size:12pt;font-size:1rem;color: #777777;margin: 1rem 0;}' +
        'div, p, ul, ol, dl, li {margin:0;}' +
        'blockquote, table, pre, code {margin:8px 0;}' +
        'ul, ol {padding-left:32px;padding-left:2rem;}' +
        'ol.wiz-list-level1 > li {list-style-type:decimal;}' +
        'ol.wiz-list-level2 > li {list-style-type:lower-latin;}' +
        'ol.wiz-list-level3 > li {list-style-type:lower-roman;}' +
        'blockquote {padding:0 12px;padding:0 0.75rem;}' +
        'blockquote > :first-child {margin-top:0;}' +
        'blockquote > :last-child {margin-bottom:0;}' +
        'img {border:0;max-width:100%;height:auto !important;margin:2px 0;}' +
        'table {border-collapse:collapse;border:1px solid #bbbbbb;}' +
        'td, th {padding:4px 8px;border-collapse:collapse;border:1px solid #bbbbbb;min-height:28px;word-break:break-all;box-sizing: border-box;}' +
        '.wiz-hide {display:none !important;}'
    };
    
var WizStyle = {
    insertDefaultStyle: function (isReplace, customCss) {
        WizStyle.replaceStyleById(CONST.ID.WIZ_DEFAULT_STYLE, DefaultStyle.common, isReplace);
        if (!customCss) {
            return;
        }
        var css, k, hasCustomCss = false;
        if (typeof customCss === 'string') {
            css = customCss;
            hasCustomCss = true;
        } else {
            css = 'html, body{';
            for (k in customCss) {
                if (customCss.hasOwnProperty(k)) {
                    if (k.toLowerCase() === 'font-family') {
                        css += (k + ':' + customCss[k] + ',' + DefaultFont + ';');
                    } else {
                        css += (k + ':' + customCss[k] + ';');
                    }
                    hasCustomCss = true;
                }
            }
            css += '}';
        }

        if (hasCustomCss) {
            WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE}, css);
        }
    },
    insertCustomStyle: function (id, customCss, isTemp) {
        if (!id || !customCss) {
            return;
        }
        var options = {id: id};
        if (isTemp) {
            options.name = CONST.NAME.TMP_STYLE;
        }
        WizStyle.insertStyle(options, customCss);
    },
    insertStyle: function (options, css) {
        var s = ENV.doc.createElement('style');
        if (options.name) {
            s.setAttribute('name', options.name);
        }
        if (options.id) {
            s.setAttribute('id', options.id);
            WizStyle.removeStyleById(options.id);
        }
        ENV.doc.getElementsByTagName('HEAD')[0].insertBefore(s, null);
        s.innerHTML = css;
        return s;
    },
    insertTmpEditorStyle: function () {
        WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE},
            TmpCommonStyle.body + TmpCommonStyle.table +
            TmpEditorStyle.imageResize + TmpEditorStyle.selectPlugin + TmpEditorStyle.table);

        if (ENV.client.type.isIOS && ENV.client.type.isPhone) {
            WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE}, TmpEditorStyle.iosPhone);
        } else if (ENV.client.type.isIOS && ENV.client.type.isPad) {
            WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE}, TmpEditorStyle.iosPad);
        }
        if (ENV.client.type.isMac) {
            WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE}, TmpCommonStyle.blockScroll);
        }
    },
    insertTmpReaderStyle: function () {
        WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE},
            TmpCommonStyle.body + TmpCommonStyle.table + TmpCommonStyle.code +
            TmpReaderStyle.code);
        if (ENV.client.type.isIOS) {
            WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE}, TmpReaderStyle.ios);
        }
        if (ENV.client.type.isMac) {
            WizStyle.insertStyle({name: CONST.NAME.TMP_STYLE}, TmpCommonStyle.blockScroll);
        }
    },

    removeFormat: function (removeAll, isRemoveColor, isRemoveAllStyles) {
        var i, element,
            links, link, styles, style;
        try {
            if (removeAll) {
                ENV.doc.execCommand('SelectAll');
            }
            if (isRemoveColor) {
                ENV.doc.execCommand('RemoveFormat');
            }
            if (removeAll) {
                element = ENV.doc.documentElement;
                if (element) {
                    if (isRemoveAllStyles) {
                        removeClass(element);
                        removeStyle(element);
                    }
                }
            }
            if (isRemoveAllStyles) {
                links = ENV.doc.getElementsByTagName("link");
                for (i = links.length - 1; i >= 0; i--) {
                    link = links[i];
                    if (link.getAttribute("rel") === "stylesheet" && !isWizDom(link)) {
                        link.remove();
                    }
                }
                styles = ENV.doc.getElementsByTagName("style");
                for (i = styles.length - 1; i >= 0; i--) {
                    style = styles[i];
                    if (!isWizDom(style)) {
                        style.remove();
                    }
                }
            }
        } catch (err) {
            console.error(err);
            return;
        }

        function isWizDom (dom) {
            return !!dom && dom.nodeType === 1 && (
                    domUtils.isTag(dom, [CONST.TAG.TMP_TAG, CONST.TAG.TMP_PLUGIN_TAG]) ||
                    dom.getAttribute('name') === CONST.NAME.TMP_STYLE ||
                    /^wiz[_-]/i.test(dom.id) ||
                    /(^| )wiz[_-]/i.test(dom.className)
                );
        }

        function removeStyle (elem) {
            if (!elem) {
                return;
            }
            if (domUtils.hasClass(elem, CONST.CLASS.CODE_CONTAINER)) {
                return;
            }
            if (!isWizDom(elem)) {
                elem.removeAttribute('style');
                if (!!elem.style.cssText) {
                    elem.style.cssText = '';
                }
            }
            var children = elem.children;
            if (!children) {
                return;
            }
            for (var i = 0; i < children.length; i++) {
                if (children[i].nodeType === 1) {
                    removeStyle(children[i]);
                }
            }
        }

        function removeClass (elem) {
            if (!elem) {
                return;
            }
            if (domUtils.hasClass(elem, CONST.CLASS.CODE_CONTAINER)) {
                return;
            }
            if (!isWizDom(elem)) {
                domUtils.attr(elem, {"class": null});
            }
            var children = elem.children;
            if (!children) {
                return;
            }
            for (var i = 0; i < children.length; i++) {
                if (children[i].nodeType === 1) {
                    removeClass(children[i]);
                }
            }
        }

    },
    removeStyleById: function (id) {
        var s = ENV.doc.getElementById(id);
        if (s && domUtils.isTag(s, ['style', 'link'])) {
            domUtils.remove(s);
        }
    },
    removeStyleByName: function (name) {
        var sList = ENV.doc.getElementsByName(name);
        var i, s;
        for (i = sList.length - 1; i >= 0; i--) {
            s = sList[i];
            if (s && domUtils.isTag(s, ['style', 'link'])) {
                domUtils.remove(s);
            }
        }
    },
    replaceStyleById: function (id, css, onlyReplace) {
        //onlyReplace = true 则 只进行替换， 如无同id 的元素，不进行任何操作
        onlyReplace = !!onlyReplace;

        var s = ENV.doc.getElementById(id);
        if (!s && !onlyReplace) {
            s = ENV.doc.createElement('style');
            s.id = id;
            ENV.doc.getElementsByTagName('HEAD')[0].insertBefore(s, null);
        }
        if (s) {
            s.innerHTML = css;
        }
    }

};

module.exports = WizStyle;

},{"../domUtils/domBase":28,"./const":18,"./env":20}],26:[function(require,module,exports){
/**
 * 专门用于记录用户行为的 log
 */
var ENV = require('../common/env');

var ActionId = {
    ClickAcceptFromAmendInfo: 'ClickAcceptFromAmendInfo',
    ClickRefuseFromAmendInfo: 'ClickRefuseFromAmendInfo'
};

var wizUserAction = {
    save: function (id) {
        if (ENV.client.type.isWin) {
            try {
                if (external && external.LogAction) {
                    external.LogAction(id);
                }
            } catch (e) {
                console.error(e.toString());
            }
        }
    }
};

var UserAction = {
    ActionId: ActionId,
    save: wizUserAction.save
};

module.exports = UserAction;
},{"../common/env":20}],27:[function(require,module,exports){
/**
 * 兼容 ES6 将 require 替换为 _require
 */
(function e(t,n,r){function s(o,u){if(!n[o]){if(!t[o]){var a=typeof _require=="function"&&_require;if(!u&&a)return a(o,!0);if(i)return i(o,!0);var f=new Error("Cannot find module '"+o+"'");throw f.code="MODULE_NOT_FOUND",f}var l=n[o]={exports:{}};t[o][0].call(l.exports,function(e){var n=t[o][1][e];return s(n?n:e)},l,l.exports,e,t,n,r)}return n[o].exports}var i=typeof _require=="function"&&_require;for(var o=0;o<r.length;o++)s(r[o]);return s})({1:[function(_require,module,exports){
    /**
     * 默认配置
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var FilterCSS = _require('cssfilter').FilterCSS;
    var getDefaultCSSWhiteList = _require('cssfilter').getDefaultWhiteList;
    var _ = _require('./util');

// 默认白名单
    function getDefaultWhiteList () {
        return {
            a:      ['target', 'href', 'title'],
            abbr:   ['title'],
            address: [],
            area:   ['shape', 'coords', 'href', 'alt'],
            article: [],
            aside:  [],
            audio:  ['autoplay', 'controls', 'loop', 'preload', 'src'],
            b:      [],
            bdi:    ['dir'],
            bdo:    ['dir'],
            big:    [],
            blockquote: ['cite'],
            br:     [],
            caption: [],
            center: [],
            cite:   [],
            code:   [],
            col:    ['align', 'valign', 'span', 'width'],
            colgroup: ['align', 'valign', 'span', 'width'],
            dd:     [],
            del:    ['datetime'],
            details: ['open'],
            div:    [],
            dl:     [],
            dt:     [],
            em:     [],
            font:   ['color', 'size', 'face'],
            footer: [],
            h1:     [],
            h2:     [],
            h3:     [],
            h4:     [],
            h5:     [],
            h6:     [],
            header: [],
            hr:     [],
            i:      [],
            img:    ['src', 'alt', 'title', 'width', 'height'],
            ins:    ['datetime'],
            li:     [],
            mark:   [],
            nav:    [],
            ol:     [],
            p:      [],
            pre:    [],
            s:      [],
            section:[],
            small:  [],
            span:   [],
            sub:    [],
            sup:    [],
            strong: [],
            table:  ['width', 'border', 'align', 'valign'],
            tbody:  ['align', 'valign'],
            td:     ['width', 'rowspan', 'colspan', 'align', 'valign'],
            tfoot:  ['align', 'valign'],
            th:     ['width', 'rowspan', 'colspan', 'align', 'valign'],
            thead:  ['align', 'valign'],
            tr:     ['rowspan', 'align', 'valign'],
            tt:     [],
            u:      [],
            ul:     [],
            video:  ['autoplay', 'controls', 'loop', 'preload', 'src', 'height', 'width']
        };
    }

// 默认CSS Filter
    var defaultCSSFilter = new FilterCSS();

    /**
     * 匹配到标签时的处理方法
     *
     * @param {String} tag
     * @param {String} html
     * @param {Object} options
     * @return {String}
     */
    function onTag (tag, html, options) {
        // do nothing
    }

    /**
     * 匹配到不在白名单上的标签时的处理方法
     *
     * @param {String} tag
     * @param {String} html
     * @param {Object} options
     * @return {String}
     */
    function onIgnoreTag (tag, html, options) {
        // do nothing
    }

    /**
     * 匹配到标签属性时的处理方法
     *
     * @param {String} tag
     * @param {String} name
     * @param {String} value
     * @return {String}
     */
    function onTagAttr (tag, name, value) {
        // do nothing
    }

    /**
     * 匹配到不在白名单上的标签属性时的处理方法
     *
     * @param {String} tag
     * @param {String} name
     * @param {String} value
     * @return {String}
     */
    function onIgnoreTagAttr (tag, name, value) {
        // do nothing
    }

    /**
     * HTML转义
     *
     * @param {String} html
     */
    function escapeHtml (html) {
        return html.replace(REGEXP_LT, '&lt;').replace(REGEXP_GT, '&gt;');
    }

    /**
     * 安全的标签属性值
     *
     * @param {String} tag
     * @param {String} name
     * @param {String} value
     * @param {Object} cssFilter
     * @return {String}
     */
    function safeAttrValue (tag, name, value, cssFilter) {
        // 转换为友好的属性值，再做判断
        value = friendlyAttrValue(value);

        if (name === 'href' || name === 'src') {
            // 过滤 href 和 src 属性
            // 仅允许 http:// | https:// | mailto: | / | # 开头的地址
            value = _.trim(value);
            if (value === '#') return '#';
            if (!(value.substr(0, 7) === 'http://' ||
                    value.substr(0, 8) === 'https://' ||
                    value.substr(0, 7) === 'mailto:' ||
                    value[0] === '#' ||
                    value[0] === '/')) {
                return '';
            }
        } else if (name === 'background') {
            // 过滤 background 属性 （这个xss漏洞较老了，可能已经不适用）
            // javascript:
            REGEXP_DEFAULT_ON_TAG_ATTR_4.lastIndex = 0;
            if (REGEXP_DEFAULT_ON_TAG_ATTR_4.test(value)) {
                return '';
            }
        } else if (name === 'style') {
            // /*注释*/
            /*REGEXP_DEFAULT_ON_TAG_ATTR_3.lastIndex = 0;
            if (REGEXP_DEFAULT_ON_TAG_ATTR_3.test(value)) {
              return '';
            }*/
            // expression()
            REGEXP_DEFAULT_ON_TAG_ATTR_7.lastIndex = 0;
            if (REGEXP_DEFAULT_ON_TAG_ATTR_7.test(value)) {
                return '';
            }
            // url()
            REGEXP_DEFAULT_ON_TAG_ATTR_8.lastIndex = 0;
            if (REGEXP_DEFAULT_ON_TAG_ATTR_8.test(value)) {
                REGEXP_DEFAULT_ON_TAG_ATTR_4.lastIndex = 0;
                if (REGEXP_DEFAULT_ON_TAG_ATTR_4.test(value)) {
                    return '';
                }
            }
            if (cssFilter !== false) {
                cssFilter = cssFilter || defaultCSSFilter;
                value = cssFilter.process(value);
            }
        }

        // 输出时需要转义<>"
        value = escapeAttrValue(value);
        return value;
    }

// 正则表达式
    var REGEXP_LT = /</g;
    var REGEXP_GT = />/g;
    var REGEXP_QUOTE = /"/g;
    var REGEXP_QUOTE_2 = /&quot;/g;
    var REGEXP_ATTR_VALUE_1 = /&#([a-zA-Z0-9]*);?/img;
    var REGEXP_ATTR_VALUE_COLON = /&colon;?/img;
    var REGEXP_ATTR_VALUE_NEWLINE = /&newline;?/img;
    var REGEXP_DEFAULT_ON_TAG_ATTR_3 = /\/\*|\*\//mg;
    var REGEXP_DEFAULT_ON_TAG_ATTR_4 = /((j\s*a\s*v\s*a|v\s*b|l\s*i\s*v\s*e)\s*s\s*c\s*r\s*i\s*p\s*t\s*|m\s*o\s*c\s*h\s*a)\:/ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_5 = /^[\s"'`]*(d\s*a\s*t\s*a\s*)\:/ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_6 = /^[\s"'`]*(d\s*a\s*t\s*a\s*)\:\s*image\//ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_7 = /e\s*x\s*p\s*r\s*e\s*s\s*s\s*i\s*o\s*n\s*\(.*/ig;
    var REGEXP_DEFAULT_ON_TAG_ATTR_8 = /u\s*r\s*l\s*\(.*/ig;

    /**
     * 对双引号进行转义
     *
     * @param {String} str
     * @return {String} str
     */
    function escapeQuote (str) {
        return str.replace(REGEXP_QUOTE, '&quot;');
    }

    /**
     * 对双引号进行转义
     *
     * @param {String} str
     * @return {String} str
     */
    function unescapeQuote (str) {
        return str.replace(REGEXP_QUOTE_2, '"');
    }

    /**
     * 对html实体编码进行转义
     *
     * @param {String} str
     * @return {String}
     */
    function escapeHtmlEntities (str) {
        return str.replace(REGEXP_ATTR_VALUE_1, function replaceUnicode (str, code) {
            return (code[0] === 'x' || code[0] === 'X')
                ? String.fromCharCode(parseInt(code.substr(1), 16))
                : String.fromCharCode(parseInt(code, 10));
        });
    }

    /**
     * 对html5新增的危险实体编码进行转义
     *
     * @param {String} str
     * @return {String}
     */
    function escapeDangerHtml5Entities (str) {
        return str.replace(REGEXP_ATTR_VALUE_COLON, ':')
            .replace(REGEXP_ATTR_VALUE_NEWLINE, ' ');
    }

    /**
     * 清除不可见字符
     *
     * @param {String} str
     * @return {String}
     */
    function clearNonPrintableCharacter (str) {
        var str2 = '';
        for (var i = 0, len = str.length; i < len; i++) {
            str2 += str.charCodeAt(i) < 32 ? ' ' : str.charAt(i);
        }
        return _.trim(str2);
    }

    /**
     * 将标签的属性值转换成一般字符，便于分析
     *
     * @param {String} str
     * @return {String}
     */
    function friendlyAttrValue (str) {
        str = unescapeQuote(str);             // 双引号
        str = escapeHtmlEntities(str);         // 转换HTML实体编码
        str = escapeDangerHtml5Entities(str);  // 转换危险的HTML5新增实体编码
        str = clearNonPrintableCharacter(str); // 清除不可见字符
        return str;
    }

    /**
     * 转义用于输出的标签属性值
     *
     * @param {String} str
     * @return {String}
     */
    function escapeAttrValue (str) {
        str = escapeQuote(str);
        str = escapeHtml(str);
        return str;
    }

    /**
     * 去掉不在白名单中的标签onIgnoreTag处理方法
     */
    function onIgnoreTagStripAll () {
        return '';
    }

    /**
     * 删除标签体
     *
     * @param {array} tags 要删除的标签列表
     * @param {function} next 对不在列表中的标签的处理函数，可选
     */
    function StripTagBody (tags, next) {
        if (typeof(next) !== 'function') {
            next = function () {};
        }

        var isRemoveAllTag = !Array.isArray(tags);
        function isRemoveTag (tag) {
            if (isRemoveAllTag) return true;
            return (_.indexOf(tags, tag) !== -1);
        }

        var removeList = [];   // 要删除的位置范围列表
        var posStart = false;  // 当前标签开始位置

        return {
            onIgnoreTag: function (tag, html, options) {
                if (isRemoveTag(tag)) {
                    if (options.isClosing) {
                        var ret = '[/removed]';
                        var end = options.position + ret.length;
                        removeList.push([posStart !== false ? posStart : options.position, end]);
                        posStart = false;
                        return ret;
                    } else {
                        if (!posStart) {
                            posStart = options.position;
                        }
                        return '[removed]';
                    }
                } else {
                    return next(tag, html, options);
                }
            },
            remove: function (html) {
                var rethtml = '';
                var lastPos = 0;
                _.forEach(removeList, function (pos) {
                    rethtml += html.slice(lastPos, pos[0]);
                    lastPos = pos[1];
                });
                rethtml += html.slice(lastPos);
                return rethtml;
            }
        };
    }

    /**
     * 去除备注标签
     *
     * @param {String} html
     * @return {String}
     */
    function stripCommentTag (html) {
        return html.replace(STRIP_COMMENT_TAG_REGEXP, '');
    }
    var STRIP_COMMENT_TAG_REGEXP = /<!--[\s\S]*?-->/g;

    /**
     * 去除不可见字符
     *
     * @param {String} html
     * @return {String}
     */
    function stripBlankChar (html) {
        var chars = html.split('');
        chars = chars.filter(function (char) {
            var c = char.charCodeAt(0);
            if (c === 127) return false;
            if (c <= 31) {
                if (c === 10 || c === 13) return true;
                return false;
            }
            return true;
        });
        return chars.join('');
    }


    exports.whiteList = getDefaultWhiteList();
    exports.getDefaultWhiteList = getDefaultWhiteList;
    exports.onTag = onTag;
    exports.onIgnoreTag = onIgnoreTag;
    exports.onTagAttr = onTagAttr;
    exports.onIgnoreTagAttr = onIgnoreTagAttr;
    exports.safeAttrValue = safeAttrValue;
    exports.escapeHtml = escapeHtml;
    exports.escapeQuote = escapeQuote;
    exports.unescapeQuote = unescapeQuote;
    exports.escapeHtmlEntities = escapeHtmlEntities;
    exports.escapeDangerHtml5Entities = escapeDangerHtml5Entities;
    exports.clearNonPrintableCharacter = clearNonPrintableCharacter;
    exports.friendlyAttrValue = friendlyAttrValue;
    exports.escapeAttrValue = escapeAttrValue;
    exports.onIgnoreTagStripAll = onIgnoreTagStripAll;
    exports.StripTagBody = StripTagBody;
    exports.stripCommentTag = stripCommentTag;
    exports.stripBlankChar = stripBlankChar;
    exports.cssFilter = defaultCSSFilter;
    exports.getDefaultCSSWhiteList = getDefaultCSSWhiteList;

},{"./util":4,"cssfilter":8}],2:[function(_require,module,exports){
    /**
     * 模块入口
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var DEFAULT = _require('./default');
    var parser = _require('./parser');
    var FilterXSS = _require('./xss');


    /**
     * XSS过滤
     *
     * @param {String} html 要过滤的HTML代码
     * @param {Object} options 选项：whiteList, onTag, onTagAttr, onIgnoreTag, onIgnoreTagAttr, safeAttrValue, escapeHtml
     * @return {String}
     */
    function filterXSS (html, options) {
        var xss = new FilterXSS(options);
        return xss.process(html);
    }


// 输出
    exports = module.exports = filterXSS;
    exports.FilterXSS = FilterXSS;
    for (var i in DEFAULT) exports[i] = DEFAULT[i];
    for (var i in parser) exports[i] = parser[i];


// 在浏览器端使用
    if (typeof window !== 'undefined') {
        window.filterXSS = module.exports;
    }

},{"./default":1,"./parser":3,"./xss":5}],3:[function(_require,module,exports){
    /**
     * 简单 HTML Parser
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var _ = _require('./util');

    /**
     * 获取标签的名称
     *
     * @param {String} html 如：'<a hef="#">'
     * @return {String}
     */
    function getTagName (html) {
        var i = html.indexOf(' ');
        if (i === -1) {
            var tagName = html.slice(1, -1);
        } else {
            var tagName = html.slice(1, i + 1);
        }
        tagName = _.trim(tagName).toLowerCase();
        if (tagName.slice(0, 1) === '/') tagName = tagName.slice(1);
        if (tagName.slice(-1) === '/') tagName = tagName.slice(0, -1);
        return tagName;
    }

    /**
     * 是否为闭合标签
     *
     * @param {String} html 如：'<a hef="#">'
     * @return {Boolean}
     */
    function isClosing (html) {
        return (html.slice(0, 2) === '</');
    }

    /**
     * 分析HTML代码，调用相应的函数处理，返回处理后的HTML
     *
     * @param {String} html
     * @param {Function} onTag 处理标签的函数
     *   参数格式： function (sourcePosition, position, tag, html, isClosing)
     * @param {Function} escapeHtml 对HTML进行转义的函数
     * @return {String}
     */
    function parseTag (html, onTag, escapeHtml) {
        'user strict';

        var rethtml = '';        // 待返回的HTML
        var lastPos = 0;         // 上一个标签结束位置
        var tagStart = false;    // 当前标签开始位置
        var quoteStart = false;  // 引号开始位置
        var currentPos = 0;      // 当前位置
        var len = html.length;   // HTML长度
        var currentHtml = '';    // 当前标签的HTML代码
        var currentTagName = ''; // 当前标签的名称

        // 逐个分析字符
        for (currentPos = 0; currentPos < len; currentPos++) {
            var c = html.charAt(currentPos);
            if (tagStart === false) {
                if (c === '<') {
                    tagStart = currentPos;
                    continue;
                }
            } else {
                if (quoteStart === false) {
                    if (c === '<') {
                        rethtml += escapeHtml(html.slice(lastPos, currentPos));
                        tagStart = currentPos;
                        lastPos = currentPos;
                        continue;
                    }
                    if (c === '>') {
                        rethtml += escapeHtml(html.slice(lastPos, tagStart));
                        currentHtml = html.slice(tagStart, currentPos + 1);
                        currentTagName = getTagName(currentHtml);
                        rethtml += onTag(tagStart,
                            rethtml.length,
                            currentTagName,
                            currentHtml,
                            isClosing(currentHtml));
                        lastPos = currentPos + 1;
                        tagStart = false;
                        continue;
                    }
                    // HTML标签内的引号仅当前一个字符是等于号时才有效
                    if ((c === '"' || c === "'") && html.charAt(currentPos - 1) === '=') {
                        quoteStart = c;
                        continue;
                    }
                } else {
                    if (c === quoteStart) {
                        quoteStart = false;
                        continue;
                    }
                }
            }
        }
        if (lastPos < html.length) {
            rethtml += escapeHtml(html.substr(lastPos));
        }

        return rethtml;
    }

// 不符合属性名称规则的正则表达式
    var REGEXP_ATTR_NAME = /[^a-zA-Z0-9_:\.\-]/img;

    /**
     * 分析标签HTML代码，调用相应的函数处理，返回HTML
     *
     * @param {String} html 如标签'<a href="#" target="_blank">' 则为 'href="#" target="_blank"'
     * @param {Function} onAttr 处理属性值的函数
     *   函数格式： function (name, value)
     * @return {String}
     */
    function parseAttr (html, onAttr) {
        'user strict';

        var lastPos = 0;        // 当前位置
        var retAttrs = [];      // 待返回的属性列表
        var tmpName = false;    // 临时属性名称
        var len = html.length;  // HTML代码长度

        function addAttr (name, value) {
            name = _.trim(name);
            name = name.replace(REGEXP_ATTR_NAME, '').toLowerCase();
            if (name.length < 1) return;
            var ret = onAttr(name, value || '');
            if (ret) retAttrs.push(ret);
        };

        // 逐个分析字符
        for (var i = 0; i < len; i++) {
            var c = html.charAt(i);
            var v, j;
            if (tmpName === false && c === '=') {
                tmpName = html.slice(lastPos, i);
                lastPos = i + 1;
                continue;
            }
            if (tmpName !== false) {
                // HTML标签内的引号仅当前一个字符是等于号时才有效
                if (i === lastPos && (c === '"' || c === "'") && html.charAt(i - 1) === '=') {
                    j = html.indexOf(c, i + 1);
                    if (j === -1) {
                        break;
                    } else {
                        v = _.trim(html.slice(lastPos + 1, j));
                        addAttr(tmpName, v);
                        tmpName = false;
                        i = j;
                        lastPos = i + 1;
                        continue;
                    }
                }
            }
            if (/\s|\n|\t/.test(c)) {
                if (tmpName === false) {
                    j = findNextEqual(html, i);
                    if (j === -1) {
                        v = _.trim(html.slice(lastPos, i));
                        addAttr(v);
                        tmpName = false;
                        lastPos = i + 1;
                        continue;
                    } else {
                        i = j - 1;
                        continue;
                    }
                } else {
                    j = findBeforeEqual(html, i - 1);
                    if (j === -1) {
                        v = _.trim(html.slice(lastPos, i));
                        v = stripQuoteWrap(v);
                        addAttr(tmpName, v);
                        tmpName = false;
                        lastPos = i + 1;
                        continue;
                    } else {
                        continue;
                    }
                }
            }
        }

        if (lastPos < html.length) {
            if (tmpName === false) {
                addAttr(html.slice(lastPos));
            } else {
                addAttr(tmpName, stripQuoteWrap(_.trim(html.slice(lastPos))));
            }
        }

        return _.trim(retAttrs.join(' '));
    }

    function findNextEqual (str, i) {
        for (; i < str.length; i++) {
            var c = str[i];
            if (c === ' ') continue;
            if (c === '=') return i;
            return -1;
        }
    }

    function findBeforeEqual (str, i) {
        for (; i > 0; i--) {
            var c = str[i];
            if (c === ' ') continue;
            if (c === '=') return i;
            return -1;
        }
    }

    function isQuoteWrapString (text) {
        if ((text[0] === '"' && text[text.length - 1] === '"') ||
            (text[0] === '\'' && text[text.length - 1] === '\'')) {
            return true;
        } else {
            return false;
        }
    };

    function stripQuoteWrap (text) {
        if (isQuoteWrapString(text)) {
            return text.substr(1, text.length - 2);
        } else {
            return text;
        }
    };


    exports.parseTag = parseTag;
    exports.parseAttr = parseAttr;

},{"./util":4}],4:[function(_require,module,exports){
    module.exports = {
        indexOf: function (arr, item) {
            var i, j;
            if (Array.prototype.indexOf) {
                return arr.indexOf(item);
            }
            for (i = 0, j = arr.length; i < j; i++) {
                if (arr[i] === item) {
                    return i;
                }
            }
            return -1;
        },
        forEach: function (arr, fn, scope) {
            var i, j;
            if (Array.prototype.forEach) {
                return arr.forEach(fn, scope);
            }
            for (i = 0, j = arr.length; i < j; i++) {
                fn.call(scope, arr[i], i, arr);
            }
        },
        trim: function (str) {
            if (String.prototype.trim) {
                return str.trim();
            }
            return str.replace(/(^\s*)|(\s*$)/g, '');
        }
    };

},{}],5:[function(_require,module,exports){
    /**
     * 过滤XSS
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var FilterCSS = _require('cssfilter').FilterCSS;
    var DEFAULT = _require('./default');
    var parser = _require('./parser');
    var parseTag = parser.parseTag;
    var parseAttr = parser.parseAttr;
    var _ = _require('./util');


    /**
     * 返回值是否为空
     *
     * @param {Object} obj
     * @return {Boolean}
     */
    function isNull (obj) {
        return (obj === undefined || obj === null);
    }

    /**
     * 取标签内的属性列表字符串
     *
     * @param {String} html
     * @return {Object}
     *   - {String} html
     *   - {Boolean} closing
     */
    function getAttrs (html) {
        var i = html.indexOf(' ');
        if (i === -1) {
            return {
                html:    '',
                closing: (html[html.length - 2] === '/')
            };
        }
        html = _.trim(html.slice(i + 1, -1));
        var isClosing = (html[html.length - 1] === '/');
        if (isClosing) html = _.trim(html.slice(0, -1));
        return {
            html:    html,
            closing: isClosing
        };
    }

    /**
     * 浅拷贝对象
     *
     * @param {Object} obj
     * @return {Object}
     */
    function shallowCopyObject (obj) {
        var ret = {};
        for (var i in obj) {
            ret[i] = obj[i];
        }
        return ret;
    }

    /**
     * XSS过滤对象
     *
     * @param {Object} options
     *   选项：whiteList, onTag, onTagAttr, onIgnoreTag,
     *        onIgnoreTagAttr, safeAttrValue, escapeHtml
     *        stripIgnoreTagBody, allowCommentTag, stripBlankChar
     *        css{whiteList, onAttr, onIgnoreAttr} css=false表示禁用cssfilter
     */
    function FilterXSS (options) {
        options = shallowCopyObject(options || {});

        if (options.stripIgnoreTag) {
            if (options.onIgnoreTag) {
                console.error('Notes: cannot use these two options "stripIgnoreTag" and "onIgnoreTag" at the same time');
            }
            options.onIgnoreTag = DEFAULT.onIgnoreTagStripAll;
        }

        options.whiteList = options.whiteList || DEFAULT.whiteList;
        options.onTag = options.onTag || DEFAULT.onTag;
        options.onTagAttr = options.onTagAttr || DEFAULT.onTagAttr;
        options.onIgnoreTag = options.onIgnoreTag || DEFAULT.onIgnoreTag;
        options.onIgnoreTagAttr = options.onIgnoreTagAttr || DEFAULT.onIgnoreTagAttr;
        options.safeAttrValue = options.safeAttrValue || DEFAULT.safeAttrValue;
        options.escapeHtml = options.escapeHtml || DEFAULT.escapeHtml;
        this.options = options;

        if (options.css === false) {
            this.cssFilter = false;
        } else {
            options.css = options.css || {};
            this.cssFilter = new FilterCSS(options.css);
        }
    }

    /**
     * 开始处理
     *
     * @param {String} html
     * @return {String}
     */
    FilterXSS.prototype.process = function (html) {
        // 兼容各种奇葩输入
        html = html || '';
        html = html.toString();
        if (!html) return '';

        var me = this;
        var options = me.options;
        var whiteList = options.whiteList;
        var onTag = options.onTag;
        var onIgnoreTag = options.onIgnoreTag;
        var onTagAttr = options.onTagAttr;
        var onIgnoreTagAttr = options.onIgnoreTagAttr;
        var safeAttrValue = options.safeAttrValue;
        var escapeHtml = options.escapeHtml;
        var cssFilter = me.cssFilter;

        // 是否清除不可见字符
        if (options.stripBlankChar) {
            html = DEFAULT.stripBlankChar(html);
        }

        // 是否禁止备注标签
        if (!options.allowCommentTag) {
            html = DEFAULT.stripCommentTag(html);
        }

        // 如果开启了stripIgnoreTagBody
        var stripIgnoreTagBody = false;
        if (options.stripIgnoreTagBody) {
            var stripIgnoreTagBody = DEFAULT.StripTagBody(options.stripIgnoreTagBody, onIgnoreTag);
            onIgnoreTag = stripIgnoreTagBody.onIgnoreTag;
        }

        var retHtml = parseTag(html, function (sourcePosition, position, tag, html, isClosing) {
            var info = {
                sourcePosition: sourcePosition,
                position:       position,
                isClosing:      isClosing,
                isWhite:        (tag in whiteList)
            };

            // 调用onTag处理
            var ret = onTag(tag, html, info);
            if (!isNull(ret)) return ret;

            // 默认标签处理方法
            if (info.isWhite) {
                // 白名单标签，解析标签属性
                // 如果是闭合标签，则不需要解析属性
                if (info.isClosing) {
                    return '</' + tag + '>';
                }

                var attrs = getAttrs(html);
                var whiteAttrList = whiteList[tag];
                var attrsHtml = parseAttr(attrs.html, function (name, value) {

                    // 调用onTagAttr处理
                    var isWhiteAttr = (_.indexOf(whiteAttrList, name) !== -1);
                    var ret = onTagAttr(tag, name, value, isWhiteAttr);
                    if (!isNull(ret)) return ret;

                    // 默认的属性处理方法
                    if (isWhiteAttr) {
                        // 白名单属性，调用safeAttrValue过滤属性值
                        value = safeAttrValue(tag, name, value, cssFilter);
                        if (value) {
                            return name + '="' + value + '"';
                        } else {
                            return name;
                        }
                    } else {
                        // 非白名单属性，调用onIgnoreTagAttr处理
                        var ret = onIgnoreTagAttr(tag, name, value, isWhiteAttr);
                        if (!isNull(ret)) return ret;
                        return;
                    }
                });

                // 构造新的标签代码
                var html = '<' + tag;
                if (attrsHtml) html += ' ' + attrsHtml;
                if (attrs.closing) html += ' /';
                html += '>';
                return html;

            } else {
                // 非白名单标签，调用onIgnoreTag处理
                var ret = onIgnoreTag(tag, html, info);
                if (!isNull(ret)) return ret;
                return escapeHtml(html);
            }

        }, escapeHtml);

        // 如果开启了stripIgnoreTagBody，需要对结果再进行处理
        if (stripIgnoreTagBody) {
            retHtml = stripIgnoreTagBody.remove(retHtml);
        }

        return retHtml;
    };


    module.exports = FilterXSS;

},{"./default":1,"./parser":3,"./util":4,"cssfilter":8}],6:[function(_require,module,exports){
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var DEFAULT = _require('./default');
    var parseStyle = _require('./parser');
    var _ = _require('./util');


    /**
     * 返回值是否为空
     *
     * @param {Object} obj
     * @return {Boolean}
     */
    function isNull (obj) {
        return (obj === undefined || obj === null);
    }

    /**
     * 浅拷贝对象
     *
     * @param {Object} obj
     * @return {Object}
     */
    function shallowCopyObject (obj) {
        var ret = {};
        for (var i in obj) {
            ret[i] = obj[i];
        }
        return ret;
    }

    /**
     * 创建CSS过滤器
     *
     * @param {Object} options
     *   - {Object} whiteList
     *   - {Object} onAttr
     *   - {Object} onIgnoreAttr
     */
    function FilterCSS (options) {
        options = shallowCopyObject(options || {});
        options.whiteList = options.whiteList || DEFAULT.whiteList;
        options.onAttr = options.onAttr || DEFAULT.onAttr;
        options.onIgnoreAttr = options.onIgnoreAttr || DEFAULT.onIgnoreAttr;
        this.options = options;
    }

    FilterCSS.prototype.process = function (css) {
        // 兼容各种奇葩输入
        css = css || '';
        css = css.toString();
        if (!css) return '';

        var me = this;
        var options = me.options;
        var whiteList = options.whiteList;
        var onAttr = options.onAttr;
        var onIgnoreAttr = options.onIgnoreAttr;

        var retCSS = parseStyle(css, function (sourcePosition, position, name, value, source) {

            var check = whiteList[name];
            var isWhite = false;
            if (check === true) isWhite = check;
            else if (typeof check === 'function') isWhite = check(value);
            else if (check instanceof RegExp) isWhite = check.test(value);
            if (isWhite !== true) isWhite = false;

            var opts = {
                position: position,
                sourcePosition: sourcePosition,
                source: source,
                isWhite: isWhite
            };

            if (isWhite) {

                var ret = onAttr(name, value, opts);
                if (isNull(ret)) {
                    return name + ':' + value;
                } else {
                    return ret;
                }

            } else {

                var ret = onIgnoreAttr(name, value, opts);
                if (!isNull(ret)) {
                    return ret;
                }

            }
        });

        return retCSS;
    };


    module.exports = FilterCSS;

},{"./default":7,"./parser":9,"./util":10}],7:[function(_require,module,exports){
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    function getDefaultWhiteList () {
        // 白名单值说明：
        // true: 允许该属性
        // Function: function (val) { } 返回true表示允许该属性，其他值均表示不允许
        // RegExp: regexp.test(val) 返回true表示允许该属性，其他值均表示不允许
        // 除上面列出的值外均表示不允许
        var whiteList = {};

        whiteList['align-content'] = false; // default: auto
        whiteList['align-items'] = false; // default: auto
        whiteList['align-self'] = false; // default: auto
        whiteList['alignment-adjust'] = false; // default: auto
        whiteList['alignment-baseline'] = false; // default: baseline
        whiteList['all'] = false; // default: depending on individual properties
        whiteList['anchor-point'] = false; // default: none
        whiteList['animation'] = false; // default: depending on individual properties
        whiteList['animation-delay'] = false; // default: 0
        whiteList['animation-direction'] = false; // default: normal
        whiteList['animation-duration'] = false; // default: 0
        whiteList['animation-fill-mode'] = false; // default: none
        whiteList['animation-iteration-count'] = false; // default: 1
        whiteList['animation-name'] = false; // default: none
        whiteList['animation-play-state'] = false; // default: running
        whiteList['animation-timing-function'] = false; // default: ease
        whiteList['azimuth'] = false; // default: center
        whiteList['backface-visibility'] = false; // default: visible
        whiteList['background'] = true; // default: depending on individual properties
        whiteList['background-attachment'] = true; // default: scroll
        whiteList['background-clip'] = true; // default: border-box
        whiteList['background-color'] = true; // default: transparent
        whiteList['background-image'] = true; // default: none
        whiteList['background-origin'] = true; // default: padding-box
        whiteList['background-position'] = true; // default: 0% 0%
        whiteList['background-repeat'] = true; // default: repeat
        whiteList['background-size'] = true; // default: auto
        whiteList['baseline-shift'] = false; // default: baseline
        whiteList['binding'] = false; // default: none
        whiteList['bleed'] = false; // default: 6pt
        whiteList['bookmark-label'] = false; // default: content()
        whiteList['bookmark-level'] = false; // default: none
        whiteList['bookmark-state'] = false; // default: open
        whiteList['border'] = true; // default: depending on individual properties
        whiteList['border-bottom'] = true; // default: depending on individual properties
        whiteList['border-bottom-color'] = true; // default: current color
        whiteList['border-bottom-left-radius'] = true; // default: 0
        whiteList['border-bottom-right-radius'] = true; // default: 0
        whiteList['border-bottom-style'] = true; // default: none
        whiteList['border-bottom-width'] = true; // default: medium
        whiteList['border-collapse'] = true; // default: separate
        whiteList['border-color'] = true; // default: depending on individual properties
        whiteList['border-image'] = true; // default: none
        whiteList['border-image-outset'] = true; // default: 0
        whiteList['border-image-repeat'] = true; // default: stretch
        whiteList['border-image-slice'] = true; // default: 100%
        whiteList['border-image-source'] = true; // default: none
        whiteList['border-image-width'] = true; // default: 1
        whiteList['border-left'] = true; // default: depending on individual properties
        whiteList['border-left-color'] = true; // default: current color
        whiteList['border-left-style'] = true; // default: none
        whiteList['border-left-width'] = true; // default: medium
        whiteList['border-radius'] = true; // default: 0
        whiteList['border-right'] = true; // default: depending on individual properties
        whiteList['border-right-color'] = true; // default: current color
        whiteList['border-right-style'] = true; // default: none
        whiteList['border-right-width'] = true; // default: medium
        whiteList['border-spacing'] = true; // default: 0
        whiteList['border-style'] = true; // default: depending on individual properties
        whiteList['border-top'] = true; // default: depending on individual properties
        whiteList['border-top-color'] = true; // default: current color
        whiteList['border-top-left-radius'] = true; // default: 0
        whiteList['border-top-right-radius'] = true; // default: 0
        whiteList['border-top-style'] = true; // default: none
        whiteList['border-top-width'] = true; // default: medium
        whiteList['border-width'] = true; // default: depending on individual properties
        whiteList['bottom'] = false; // default: auto
        whiteList['box-decoration-break'] = true; // default: slice
        whiteList['box-shadow'] = true; // default: none
        whiteList['box-sizing'] = true; // default: content-box
        whiteList['box-snap'] = true; // default: none
        whiteList['box-suppress'] = true; // default: show
        whiteList['break-after'] = true; // default: auto
        whiteList['break-before'] = true; // default: auto
        whiteList['break-inside'] = true; // default: auto
        whiteList['caption-side'] = false; // default: top
        whiteList['chains'] = false; // default: none
        whiteList['clear'] = true; // default: none
        whiteList['clip'] = false; // default: auto
        whiteList['clip-path'] = false; // default: none
        whiteList['clip-rule'] = false; // default: nonzero
        whiteList['color'] = true; // default: implementation dependent
        whiteList['color-interpolation-filters'] = true; // default: auto
        whiteList['column-count'] = false; // default: auto
        whiteList['column-fill'] = false; // default: balance
        whiteList['column-gap'] = false; // default: normal
        whiteList['column-rule'] = false; // default: depending on individual properties
        whiteList['column-rule-color'] = false; // default: current color
        whiteList['column-rule-style'] = false; // default: medium
        whiteList['column-rule-width'] = false; // default: medium
        whiteList['column-span'] = false; // default: none
        whiteList['column-width'] = false; // default: auto
        whiteList['columns'] = false; // default: depending on individual properties
        whiteList['contain'] = false; // default: none
        whiteList['content'] = false; // default: normal
        whiteList['counter-increment'] = false; // default: none
        whiteList['counter-reset'] = false; // default: none
        whiteList['counter-set'] = false; // default: none
        whiteList['crop'] = false; // default: auto
        whiteList['cue'] = false; // default: depending on individual properties
        whiteList['cue-after'] = false; // default: none
        whiteList['cue-before'] = false; // default: none
        whiteList['cursor'] = false; // default: auto
        whiteList['direction'] = false; // default: ltr
        whiteList['display'] = true; // default: depending on individual properties
        whiteList['display-inside'] = true; // default: auto
        whiteList['display-list'] = true; // default: none
        whiteList['display-outside'] = true; // default: inline-level
        whiteList['dominant-baseline'] = false; // default: auto
        whiteList['elevation'] = false; // default: level
        whiteList['empty-cells'] = false; // default: show
        whiteList['filter'] = false; // default: none
        whiteList['flex'] = false; // default: depending on individual properties
        whiteList['flex-basis'] = false; // default: auto
        whiteList['flex-direction'] = false; // default: row
        whiteList['flex-flow'] = false; // default: depending on individual properties
        whiteList['flex-grow'] = false; // default: 0
        whiteList['flex-shrink'] = false; // default: 1
        whiteList['flex-wrap'] = false; // default: nowrap
        whiteList['float'] = false; // default: none
        whiteList['float-offset'] = false; // default: 0 0
        whiteList['flood-color'] = false; // default: black
        whiteList['flood-opacity'] = false; // default: 1
        whiteList['flow-from'] = false; // default: none
        whiteList['flow-into'] = false; // default: none
        whiteList['font'] = true; // default: depending on individual properties
        whiteList['font-family'] = true; // default: implementation dependent
        whiteList['font-feature-settings'] = true; // default: normal
        whiteList['font-kerning'] = true; // default: auto
        whiteList['font-language-override'] = true; // default: normal
        whiteList['font-size'] = true; // default: medium
        whiteList['font-size-adjust'] = true; // default: none
        whiteList['font-stretch'] = true; // default: normal
        whiteList['font-style'] = true; // default: normal
        whiteList['font-synthesis'] = true; // default: weight style
        whiteList['font-variant'] = true; // default: normal
        whiteList['font-variant-alternates'] = true; // default: normal
        whiteList['font-variant-caps'] = true; // default: normal
        whiteList['font-variant-east-asian'] = true; // default: normal
        whiteList['font-variant-ligatures'] = true; // default: normal
        whiteList['font-variant-numeric'] = true; // default: normal
        whiteList['font-variant-position'] = true; // default: normal
        whiteList['font-weight'] = true; // default: normal
        whiteList['grid'] = false; // default: depending on individual properties
        whiteList['grid-area'] = false; // default: depending on individual properties
        whiteList['grid-auto-columns'] = false; // default: auto
        whiteList['grid-auto-flow'] = false; // default: none
        whiteList['grid-auto-rows'] = false; // default: auto
        whiteList['grid-column'] = false; // default: depending on individual properties
        whiteList['grid-column-end'] = false; // default: auto
        whiteList['grid-column-start'] = false; // default: auto
        whiteList['grid-row'] = false; // default: depending on individual properties
        whiteList['grid-row-end'] = false; // default: auto
        whiteList['grid-row-start'] = false; // default: auto
        whiteList['grid-template'] = false; // default: depending on individual properties
        whiteList['grid-template-areas'] = false; // default: none
        whiteList['grid-template-columns'] = false; // default: none
        whiteList['grid-template-rows'] = false; // default: none
        whiteList['hanging-punctuation'] = false; // default: none
        whiteList['height'] = true; // default: auto
        whiteList['hyphens'] = false; // default: manual
        whiteList['icon'] = false; // default: auto
        whiteList['image-orientation'] = false; // default: auto
        whiteList['image-resolution'] = false; // default: normal
        whiteList['ime-mode'] = false; // default: auto
        whiteList['initial-letters'] = false; // default: normal
        whiteList['inline-box-align'] = false; // default: last
        whiteList['justify-content'] = false; // default: auto
        whiteList['justify-items'] = false; // default: auto
        whiteList['justify-self'] = false; // default: auto
        whiteList['left'] = false; // default: auto
        whiteList['letter-spacing'] = true; // default: normal
        whiteList['lighting-color'] = true; // default: white
        whiteList['line-box-contain'] = false; // default: block inline replaced
        whiteList['line-break'] = false; // default: auto
        whiteList['line-grid'] = false; // default: match-parent
        whiteList['line-height'] = false; // default: normal
        whiteList['line-snap'] = false; // default: none
        whiteList['line-stacking'] = false; // default: depending on individual properties
        whiteList['line-stacking-ruby'] = false; // default: exclude-ruby
        whiteList['line-stacking-shift'] = false; // default: consider-shifts
        whiteList['line-stacking-strategy'] = false; // default: inline-line-height
        whiteList['list-style'] = true; // default: depending on individual properties
        whiteList['list-style-image'] = true; // default: none
        whiteList['list-style-position'] = true; // default: outside
        whiteList['list-style-type'] = true; // default: disc
        whiteList['margin'] = true; // default: depending on individual properties
        whiteList['margin-bottom'] = true; // default: 0
        whiteList['margin-left'] = true; // default: 0
        whiteList['margin-right'] = true; // default: 0
        whiteList['margin-top'] = true; // default: 0
        whiteList['marker-offset'] = false; // default: auto
        whiteList['marker-side'] = false; // default: list-item
        whiteList['marks'] = false; // default: none
        whiteList['mask'] = false; // default: border-box
        whiteList['mask-box'] = false; // default: see individual properties
        whiteList['mask-box-outset'] = false; // default: 0
        whiteList['mask-box-repeat'] = false; // default: stretch
        whiteList['mask-box-slice'] = false; // default: 0 fill
        whiteList['mask-box-source'] = false; // default: none
        whiteList['mask-box-width'] = false; // default: auto
        whiteList['mask-clip'] = false; // default: border-box
        whiteList['mask-image'] = false; // default: none
        whiteList['mask-origin'] = false; // default: border-box
        whiteList['mask-position'] = false; // default: center
        whiteList['mask-repeat'] = false; // default: no-repeat
        whiteList['mask-size'] = false; // default: border-box
        whiteList['mask-source-type'] = false; // default: auto
        whiteList['mask-type'] = false; // default: luminance
        whiteList['max-height'] = true; // default: none
        whiteList['max-lines'] = false; // default: none
        whiteList['max-width'] = true; // default: none
        whiteList['min-height'] = true; // default: 0
        whiteList['min-width'] = true; // default: 0
        whiteList['move-to'] = false; // default: normal
        whiteList['nav-down'] = false; // default: auto
        whiteList['nav-index'] = false; // default: auto
        whiteList['nav-left'] = false; // default: auto
        whiteList['nav-right'] = false; // default: auto
        whiteList['nav-up'] = false; // default: auto
        whiteList['object-fit'] = false; // default: fill
        whiteList['object-position'] = false; // default: 50% 50%
        whiteList['opacity'] = false; // default: 1
        whiteList['order'] = false; // default: 0
        whiteList['orphans'] = false; // default: 2
        whiteList['outline'] = false; // default: depending on individual properties
        whiteList['outline-color'] = false; // default: invert
        whiteList['outline-offset'] = false; // default: 0
        whiteList['outline-style'] = false; // default: none
        whiteList['outline-width'] = false; // default: medium
        whiteList['overflow'] = false; // default: depending on individual properties
        whiteList['overflow-wrap'] = false; // default: normal
        whiteList['overflow-x'] = false; // default: visible
        whiteList['overflow-y'] = false; // default: visible
        whiteList['padding'] = true; // default: depending on individual properties
        whiteList['padding-bottom'] = true; // default: 0
        whiteList['padding-left'] = true; // default: 0
        whiteList['padding-right'] = true; // default: 0
        whiteList['padding-top'] = true; // default: 0
        whiteList['page'] = false; // default: auto
        whiteList['page-break-after'] = false; // default: auto
        whiteList['page-break-before'] = false; // default: auto
        whiteList['page-break-inside'] = false; // default: auto
        whiteList['page-policy'] = false; // default: start
        whiteList['pause'] = false; // default: implementation dependent
        whiteList['pause-after'] = false; // default: implementation dependent
        whiteList['pause-before'] = false; // default: implementation dependent
        whiteList['perspective'] = false; // default: none
        whiteList['perspective-origin'] = false; // default: 50% 50%
        whiteList['pitch'] = false; // default: medium
        whiteList['pitch-range'] = false; // default: 50
        whiteList['play-during'] = false; // default: auto
        whiteList['position'] = false; // default: static
        whiteList['presentation-level'] = false; // default: 0
        whiteList['quotes'] = false; // default: text
        whiteList['region-fragment'] = false; // default: auto
        whiteList['resize'] = false; // default: none
        whiteList['rest'] = false; // default: depending on individual properties
        whiteList['rest-after'] = false; // default: none
        whiteList['rest-before'] = false; // default: none
        whiteList['richness'] = false; // default: 50
        whiteList['right'] = false; // default: auto
        whiteList['rotation'] = false; // default: 0
        whiteList['rotation-point'] = false; // default: 50% 50%
        whiteList['ruby-align'] = false; // default: auto
        whiteList['ruby-merge'] = false; // default: separate
        whiteList['ruby-position'] = false; // default: before
        whiteList['shape-image-threshold'] = false; // default: 0.0
        whiteList['shape-outside'] = false; // default: none
        whiteList['shape-margin'] = false; // default: 0
        whiteList['size'] = false; // default: auto
        whiteList['speak'] = false; // default: auto
        whiteList['speak-as'] = false; // default: normal
        whiteList['speak-header'] = false; // default: once
        whiteList['speak-numeral'] = false; // default: continuous
        whiteList['speak-punctuation'] = false; // default: none
        whiteList['speech-rate'] = false; // default: medium
        whiteList['stress'] = false; // default: 50
        whiteList['string-set'] = false; // default: none
        whiteList['tab-size'] = false; // default: 8
        whiteList['table-layout'] = false; // default: auto
        whiteList['text-align'] = true; // default: start
        whiteList['text-align-last'] = true; // default: auto
        whiteList['text-combine-upright'] = true; // default: none
        whiteList['text-decoration'] = true; // default: none
        whiteList['text-decoration-color'] = true; // default: currentColor
        whiteList['text-decoration-line'] = true; // default: none
        whiteList['text-decoration-skip'] = true; // default: objects
        whiteList['text-decoration-style'] = true; // default: solid
        whiteList['text-emphasis'] = true; // default: depending on individual properties
        whiteList['text-emphasis-color'] = true; // default: currentColor
        whiteList['text-emphasis-position'] = true; // default: over right
        whiteList['text-emphasis-style'] = true; // default: none
        whiteList['text-height'] = true; // default: auto
        whiteList['text-indent'] = true; // default: 0
        whiteList['text-justify'] = true; // default: auto
        whiteList['text-orientation'] = true; // default: mixed
        whiteList['text-overflow'] = true; // default: clip
        whiteList['text-shadow'] = true; // default: none
        whiteList['text-space-collapse'] = true; // default: collapse
        whiteList['text-transform'] = true; // default: none
        whiteList['text-underline-position'] = true; // default: auto
        whiteList['text-wrap'] = true; // default: normal
        whiteList['top'] = false; // default: auto
        whiteList['transform'] = false; // default: none
        whiteList['transform-origin'] = false; // default: 50% 50% 0
        whiteList['transform-style'] = false; // default: flat
        whiteList['transition'] = false; // default: depending on individual properties
        whiteList['transition-delay'] = false; // default: 0s
        whiteList['transition-duration'] = false; // default: 0s
        whiteList['transition-property'] = false; // default: all
        whiteList['transition-timing-function'] = false; // default: ease
        whiteList['unicode-bidi'] = false; // default: normal
        whiteList['vertical-align'] = false; // default: baseline
        whiteList['visibility'] = false; // default: visible
        whiteList['voice-balance'] = false; // default: center
        whiteList['voice-duration'] = false; // default: auto
        whiteList['voice-family'] = false; // default: implementation dependent
        whiteList['voice-pitch'] = false; // default: medium
        whiteList['voice-range'] = false; // default: medium
        whiteList['voice-rate'] = false; // default: normal
        whiteList['voice-stress'] = false; // default: normal
        whiteList['voice-volume'] = false; // default: medium
        whiteList['volume'] = false; // default: medium
        whiteList['white-space'] = false; // default: normal
        whiteList['widows'] = false; // default: 2
        whiteList['width'] = true; // default: auto
        whiteList['will-change'] = false; // default: auto
        whiteList['word-break'] = true; // default: normal
        whiteList['word-spacing'] = true; // default: normal
        whiteList['word-wrap'] = true; // default: normal
        whiteList['wrap-flow'] = false; // default: auto
        whiteList['wrap-through'] = false; // default: wrap
        whiteList['writing-mode'] = false; // default: horizontal-tb
        whiteList['z-index'] = false; // default: auto

        return whiteList;
    }


    /**
     * 匹配到白名单上的一个属性时
     *
     * @param {String} name
     * @param {String} value
     * @param {Object} options
     * @return {String}
     */
    function onAttr (name, value, options) {
        // do nothing
    }

    /**
     * 匹配到不在白名单上的一个属性时
     *
     * @param {String} name
     * @param {String} value
     * @param {Object} options
     * @return {String}
     */
    function onIgnoreAttr (name, value, options) {
        // do nothing
    }


    exports.whiteList = getDefaultWhiteList();
    exports.getDefaultWhiteList = getDefaultWhiteList;
    exports.onAttr = onAttr;
    exports.onIgnoreAttr = onIgnoreAttr;

},{}],8:[function(_require,module,exports){
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var DEFAULT = _require('./default');
    var FilterCSS = _require('./css');


    /**
     * XSS过滤
     *
     * @param {String} css 要过滤的CSS代码
     * @param {Object} options 选项：whiteList, onAttr, onIgnoreAttr
     * @return {String}
     */
    function filterCSS (html, options) {
        var xss = new FilterCSS(options);
        return xss.process(html);
    }


// 输出
    exports = module.exports = filterCSS;
    exports.FilterCSS = FilterCSS;
    for (var i in DEFAULT) exports[i] = DEFAULT[i];

// 在浏览器端使用
    if (typeof window !== 'undefined') {
        window.filterCSS = module.exports;
    }

},{"./css":6,"./default":7}],9:[function(_require,module,exports){
    /**
     * cssfilter
     *
     * @author 老雷<leizongmin@gmail.com>
     */

    var _ = _require('./util');


    /**
     * 解析style
     *
     * @param {String} css
     * @param {Function} onAttr 处理属性的函数
     *   参数格式： function (sourcePosition, position, name, value, source)
     * @return {String}
     */
    function parseStyle (css, onAttr) {
        css = _.trimRight(css);
        if (css[css.length - 1] !== ';') css += ';';
        var cssLength = css.length;
        var isParenthesisOpen = false;
        var lastPos = 0;
        var i = 0;
        var retCSS = '';

        function addNewAttr () {
            // 如果没有正常的闭合圆括号，则直接忽略当前属性
            if (!isParenthesisOpen) {
                var source = _.trim(css.slice(lastPos, i));
                var j = source.indexOf(':');
                if (j !== -1) {
                    var name = _.trim(source.slice(0, j));
                    var value = _.trim(source.slice(j + 1));
                    // 必须有属性名称
                    if (name) {
                        var ret = onAttr(lastPos, retCSS.length, name, value, source);
                        if (ret) retCSS += ret + '; ';
                    }
                }
            }
            lastPos = i + 1;
        }

        for (; i < cssLength; i++) {
            var c = css[i];
            if (c === '/' && css[i + 1] === '*') {
                // 备注开始
                var j = css.indexOf('*/', i + 2);
                // 如果没有正常的备注结束，则后面的部分全部跳过
                if (j === -1) break;
                // 直接将当前位置调到备注结尾，并且初始化状态
                i = j + 1;
                lastPos = i + 1;
                isParenthesisOpen = false;
            } else if (c === '(') {
                isParenthesisOpen = true;
            } else if (c === ')') {
                isParenthesisOpen = false;
            } else if (c === ';') {
                if (isParenthesisOpen) {
                    // 在圆括号里面，忽略
                } else {
                    addNewAttr();
                }
            } else if (c === '\n') {
                addNewAttr();
            }
        }

        return _.trim(retCSS);
    }

    module.exports = parseStyle;

},{"./util":10}],10:[function(_require,module,exports){
    module.exports = {
        indexOf: function (arr, item) {
            var i, j;
            if (Array.prototype.indexOf) {
                return arr.indexOf(item);
            }
            for (i = 0, j = arr.length; i < j; i++) {
                if (arr[i] === item) {
                    return i;
                }
            }
            return -1;
        },
        forEach: function (arr, fn, scope) {
            var i, j;
            if (Array.prototype.forEach) {
                return arr.forEach(fn, scope);
            }
            for (i = 0, j = arr.length; i < j; i++) {
                fn.call(scope, arr[i], i, arr);
            }
        },
        trim: function (str) {
            if (String.prototype.trim) {
                return str.trim();
            }
            return str.replace(/(^\s*)|(\s*$)/g, '');
        },
        trimRight: function (str) {
            if (String.prototype.trimRight) {
                return str.trimRight();
            }
            return str.replace(/(\s*$)/g, '');
        }
    };

},{}]},{},[2]);

module.exports = filterXSS;
},{}],28:[function(require,module,exports){
/**
 * Dom 操作工具包（基础核心包，主要都是 get 等读取操作）
 *
 */

var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils');

var defaultFontSize = 12;

var domUtils = {
    /**
     * 生成并返回当前设备 1pt 对应 px 的值
     */
    pt2px: function () {
        var pt2px;
        var span = document.createElement('span');
        span.style.visibility = 'hidden';
        span.style.position = 'absolute';
        span.style.top = 0;
        span.style.left = 0;
        span.style.fontSize = defaultFontSize + 'pt';
        ENV.doc.body.appendChild(span);
        pt2px = window.getComputedStyle(span).fontSize;
        pt2px = parseInt(pt2px, 10) / defaultFontSize;
        ENV.doc.body.removeChild(span);
        domUtils.pt2px = function () {
            return pt2px;
        };
        // console.log('1pt == ' + pt2px + 'px');
        return pt2px;
    },
    /**
     * 生成并返回当前笔记视图放大状态
     * 1. 必须有 wiz_customm_css 样式
     * 2. 强制 12pt 当作 1:1 的状态
     */
    getRootSizeRate: function () {
        if (!ENV.doc.getElementById(CONST.ID.WIZ_DEFAULT_STYLE)) {
            return 1;
        }
        var s = ENV.win.getComputedStyle(ENV.doc.body.parentNode),
            rootSize = parseFloat(s.fontSize);
        // console.log('rootSize rate: ' + (parseFloat(rootSize) / domUtils.pt2px() / 12));
        return parseFloat(rootSize) / domUtils.pt2px() / defaultFontSize;
    },
    /**
     * 添加 class name
     * @param domList
     * @param className
     */
    addClass: function (domList, className) {
        if (!domList) {
            return;
        }
        if (!!domList.nodeType) {
            domList = [domList];
        }
        var i, dom;
        for (i = domList.length - 1; i >= 0; i--) {
            dom = domList[i];
            if (dom.nodeType === 1 && !domUtils.hasClass(dom, className)) {
                dom.className = (dom.className + ' ' + className).trim();
            }
        }
    },
    /**
     * 给 DOM.getAttribute('style') 对象 设置 样式
     * @param styleStr
     * @param styleObj
     */
    appendStyle: function (styleStr, styleObj) {
        if (!styleStr) {
            return;
        }
        var styleList = styleStr.split(';'), i, j, t;
        for (i = 0, j = styleList.length; i < j; i++) {
            if (styleList[i].indexOf(':') > 0) {
                t = styleList[i].split(':');
                styleObj[t[0].trim()] = t[1].trim();
            }
        }
    },
    /**
     * 修改子节点的属性（attribute）
     * @param dom
     * @param attr
     */
    attr: function (dom, attr) {
        var key, value;
        if (!dom || !attr || dom.nodeType !== 1) {
            return;
        }
        for (key in attr) {
            if (attr.hasOwnProperty(key) && typeof key == 'string') {
                value = attr[key];
                if (!value) {
                    dom.removeAttribute(key);
                } else {
                    dom.setAttribute(key, value);
                }
            }
        }
    },
    /**
     * 判断 dom 是否为可编辑的 dom 类型
     * @param dom
     * @returns {*|boolean}
     */
    canEdit: function (dom) {
        //过滤 script、style等标签
        var filterTag = ['script', 'style'];

        return dom && (dom.nodeType == 1 || dom.nodeType == 3) &&
            (domUtils.isTag(dom, 'br') || !domUtils.isEmptyDom(dom)) && !(domUtils.getParentByTagName(dom, CONST.TAG.TMP_TAG, true, null)) && !((dom.nodeType === 1 && domUtils.isTag(dom, filterTag)) ||
                (dom.nodeType === 3 && dom.parentNode && domUtils.isTag(dom.parentNode, filterTag)));
    },
    /**
     * 清理 dom 内无用的 childNodes（主要用于 处理 剪切板的 html）
     * @param dom
     */
    childNodesFilter: function (dom) {
        if (!dom || dom.nodeType !== 1) {
            return;
        }
        var i, d;
        for (i = dom.childNodes.length - 1; i >= 0; i--) {
            d = dom.childNodes[i];
            if (d.nodeType == 1) {
                if (/link|style|script|meta/ig.test(d.nodeName)) {
                    dom.removeChild(d);
                }
                domUtils.childNodesFilter(d);
            } else if (d.nodeType != 3) {
                dom.removeChild(d);
            }
        }
    },
    /**
     * 清除 Dom 上 某一个 inline 的 style 属性
     * @param dom
     * @param styleKey
     * @param isSingle 是否只处理 dom 本身（false : 会自动处理其行级元素父节点）
     */
    clearStyle: function (dom, styleKey, isSingle) {
        removeStyle(dom, styleKey);
        if (isSingle) {
            return;
        }
        while (dom && !domUtils.isBlock(dom)) {
            removeStyle(dom, styleKey);
            dom = dom.parentNode;
        }

        function removeStyle (_dom, _styleKey) {
            _dom.style[styleKey] = '';
            if (domUtils.isTag(_dom, 'font') && _styleKey === 'color') {
                _dom.removeAttribute('color');
            }
        }
    },
    /**
     * 清除 html 字符串内 style 样式
     * @param html
     * @param whiteList
     * @returns {*|void|XML|string}
     */
    clearStyleFromHtml: function (html, whiteList) {
        var reg = /(<[^<>]* style=(['"]))((\r?\n|(?!(\2|[<>])).)*)/ig;
        var i;

        whiteList = whiteList ? whiteList : [];
        var white, index;
        for (i = 0; i < whiteList.length; i++) {
            white = whiteList[i];
            index = white.indexOf('*');
            whiteList[i] = index >= 0 ? white.substr(0, index).escapeRegex() : white.escapeRegex() + '$';
        }

        var whiteListReg = whiteList.length > 0 ? new RegExp('^(' + whiteList.join('|') + ')', 'i') : null;

        html = html.replace(reg, function (m0, m1, m2, m3) {
            var result = [m1];
            // 用 ; 分割后必须进行修正，例如：
            //font-family: -apple-system, &quot;Helvetica Neue&quot;, Arial, &quot;PingFang SC&quot;, &quot;Hiragino Sans GB&quot;, &quot;Microsoft YaHei&quot;, &quot;WenQuanYi Micro Hei&quot;, sans-serif;
            var lastStyleHasEnd = true,
                styleList = m3.split(';'), style,
                index, k, v, i, j;
            for (i = 0, j = styleList.length; i < j; i++) {
                style = styleList[i];
                index = style.indexOf(':');
                k = '';
                v = '';
                if (index > -1) {
                    k = style.substr(0, index).trim();
                    v = style.substr(index + 1).trim()
                } else if (!lastStyleHasEnd && !!style) {
                    // 避免由于 转义字符引起的 分割 &quot;
                    result.push(style, ';');
                    continue;
                }
                if (k && whiteListReg && whiteListReg.test(k)) {
                    if (/^font-size$/i.test(k)) {
                        v = domUtils.getFontSizeRem(v, {
                            useRootSize: true
                        });
                    }
                    if (!v || /^(inherit|initial|transparent)$/i.test(v)) {
                        lastStyleHasEnd = false;
                    } else {
                        result.push(k, ':', v, ';');
                        lastStyleHasEnd = false;
                    }
                } else {
                    lastStyleHasEnd = true;
                }
            }
            return result.join('');
        });
        return html;
    },
    /**
     * 复制 dom
     * @param dom
     * @param excludeInner
     */
    clone: function (dom, excludeInner) {
        var result, tmp, attList, atts, i, j, k;
        if (!dom) {
            return null;
        }
        if (excludeInner) {
            if (dom.nodeType === 3) {
                result = ENV.doc.createTextNode('');
            } else {
                result = ENV.doc.createElement(dom.tagName);
                attList = dom.attributes;
                atts = {};
                for (i = 0, j = attList.length; i < j; i++) {
                    k = attList[i];
                    if (/^id$/i.test(k.nodeName)) {
                        continue;
                    }
                    atts[k.nodeName] = k.nodeValue;
                }
                domUtils.attr(result, atts);
            }

        } else {
            if (dom.nodeType === 3) {
                result = ENV.doc.createTextNode(dom.nodeValue);
            } else {
                tmp = ENV.doc.createElement('div');
                tmp.innerHTML = dom.outerHTML;
                result = tmp.childNodes[0];
            }
        }

        return result;
    },
    /**
     * 比较 IndexList
     * @param a
     * @param b
     * @returns {number}
     */
    compareIndexList: function (a, b) {
        var i, j = Math.min(a.length, b.length), x, y;
        for (i = 0; i < j; i++) {
            x = a[i];
            y = b[i];
            if (x < y) {
                return -1;
            }
            if (x > y) {
                return 1;
            }
        }

        if (a.length < b.length) {
            return -1;
        }

        if (a.length > b.length) {
            return 1;
        }

        return 0;
    },
    /**
     * a 是否包含 b （from jQuery 1.10.2）
     * @param a
     * @param b
     * @returns {boolean}
     */
    contains: function (a, b) {
        var adown = a.nodeType === 9 ? a.documentElement : a,
            bup = b && b.parentNode;
        return a === bup || !!( bup && bup.nodeType === 1 && (
            adown.contains ?
                adown.contains(bup) :
                a.compareDocumentPosition && a.compareDocumentPosition(bup) & 16
        ));
    },
    /**
     * 根据 src 将 img 转换为 Base64
     * @param src
     */
    convertImageToBase64: function (src, width, height, callback) {
        // var xhr = new XMLHttpRequest();
        // xhr.open('GET', src, true);
        // xhr.responseType = 'blob';
        // xhr.onload = function() {
        //     var blob, read;
        //     if (/^file:/i.test(src) || this.status == 200) {
        //         blob = this.response;
        //         read= new FileReader();
        //         read.readAsDataURL(blob);
        //         read.onload = function(){
        //             // callback(canvas.toDataURL());
        //             console.log(this.result);
        //         }
        //     } else {console.log(this)}
        // };
        // xhr.send();
        var img = ENV.doc.createElement('img');
        img.onload = function () {
            var canvas = ENV.doc.createElement("canvas");
            canvas.width = width;
            canvas.height = height;
            var context = canvas.getContext("2d");
            var dx = width / img.width;
            var dy = height / img.height;
            var d = Math.max(dx, dy);
            context.scale(d, d);
            context.drawImage(img, 0, 0);
            callback(canvas.toDataURL());
            img = null;
            canvas = null;
        };
        img.src = src;
    },
    /**
     * 创建 wiz编辑器 自用的 span
     */
    createSpan: function () {
        var s = ENV.doc.createElement('span');
        s.setAttribute(CONST.ATTR.SPAN, CONST.ATTR.SPAN);
        return s;
    },
    /**
     * 设置 dom css
     * @param dom
     * @param style {{}}
     */
    css: function (dom, style) {
        if (!dom || !style || domUtils.isTag(dom, 'br')) {
            //禁止给 br 添加任何样式
            return;
        }
        var targetStyle = {};
        var k, v;
        for (k in style) {
            if (style.hasOwnProperty(k) && typeof k == 'string') {
                v = style[k];
                if (!v && v !== 0) {
                    domUtils.clearStyle(dom, k, false);
                } else {
                    targetStyle[k] = v;
                }
            }
        }

        for (k in targetStyle) {
            if (targetStyle.hasOwnProperty(k) && typeof k == 'string') {
                v = targetStyle[k];
                if (!v && v !== 0) {
                    domUtils.clearStyle(dom, k, false);
                } else if (v.toString().indexOf('!important') > 0) {
                    //对于 具有 !important 的样式需要特殊添加
                    domUtils.clearStyle(dom, k, true);
                    dom.style.cssText += ( k + ':' + v );
                } else if (k.toLowerCase() == 'font-size') {
                    //如果设置的字体与 body 默认字体 同样大小， 则扩展设置 rem
                    domUtils.clearStyle(dom, k, true);
                    v = domUtils.getFontSizeRem(v);
                    if (v) {
                        dom.style.cssText += ( k + ':' + v );
                    }
                } else {
                    dom.style[k] = v;
                }
            }
        }
    },
    /**
     * 设置 焦点
     */
    focus: function () {
        if (ENV.win.WizTemplate) {
            ENV.win.WizTemplate.focus();
        } else {
            ENV.doc.body.focus();
        }
    },
    getBlockParent: function (dom, includeSelf) {
        if (!dom) {
            return null;
        }
        return domUtils.getParentByFilter(dom, function (obj) {
            return domUtils.isBlock(obj);
        }, includeSelf);
    },
    /**
     * 获取 dom 的 计算样式
     * @param dom
     * @param name
     * @param includeParent  （Boolean 如果当前Dom 不存在指定的样式，是否递归到父节点）
     * @returns {*}
     */
    getComputedStyle: function (dom, name, includeParent) {
        if (!dom || dom.nodeType == 3 || !name) {
            return '';
        }
        var value;
        //while (includeParent && !value && dom!=ENV.doc.body) {
        while (!value) {
            if (dom.nodeType !== 1) {
                dom = dom.parentNode;
                continue;
            }

            var s = ENV.win.getComputedStyle(dom, null);
            value = s[name] || '';

            if (/^rgba?\(.*\)$/i.test(value)) {
                value = utils.rgb2Hex(value);
            }

            if (dom == ENV.doc.body || !includeParent || !!value) {
                break;
            }

            //(includeParent && !value)
            dom = dom.parentNode;
        }
        return value;
    },
    getDocType: function (doc) {
        var docType = doc.doctype;
        if (!!docType && !docType.systemId && !docType.publicId) {
            docType = '<!DOCTYPE HTML>';
        } else if (!!docType) {
            docType = '<!DOCTYPE HTML PUBLIC "' + docType.publicId + '" "' + docType.systemId + '" >';
        } else {
            docType = '<!DOCTYPE HTML>';
        }
        return docType;
    },
    /**
     * 获取 Dom 的子元素长度（同时支持 TextNode 和 Element）
     * @param dom
     * @returns {*}
     */
    getEndOffset: function (dom) {
        if (!dom) {
            return 0;
        }
        return dom.nodeType == 3 ? dom.nodeValue.length : dom.childNodes.length;
    },
    /**
     * 获取 dom 子孙元素中第一个 叶子节点
     * @param obj
     * @returns {*}
     */
    getFirstDeepChild: function (obj) {
        if (!obj) {
            return null;
        }
        var tmp = obj;
        while (tmp && tmp.childNodes && tmp.childNodes.length > 0) {
            tmp = tmp.childNodes[0];
            while (tmp && tmp.nodeType === 1 && /^wiz_/ig.test(tmp.tagName)) {
                tmp = tmp.nextSibling;
            }
            if (tmp) {
                obj = tmp;
            }
        }
        return obj;
    },
    // /**
    //  * 获取 dom 子孙元素中第一个非空的 叶子节点
    //  * @param obj
    //  * @returns {*}
    //  */
    // getFirstDeepChildNotEmpty: function (obj) {
    //     var child = domUtils.getFirstDeepChild(obj);
    //     while (child && domUtils.isEmptyDom(child)) {
    //         child = domUtils.getNextNode(child, false, obj);
    //     }
    //     return child;
    // },
    /**
     * 根据字号获取 rem 值
     * @param fontSize
     * @returns {*}
     */
    getFontSizeRem: function (fontSize, _options) {
        var fontSize = fontSize.trim();
        var size = parseFloat(fontSize);
        if (fontSize.indexOf('%') > -1) {
            return (Math.round((size / 100) * 1000)) / 1000 + 'rem';
        } else if (/^smaller$/i.test(fontSize)) {
            return '0.83rem';
        } else if (/^small$/i.test(fontSize)) {
            return '0.8125rem';
        } else if (/^x-small$/i.test(fontSize)) {
            return '0.75rem';
        } else if (/^xx-small$/i.test(fontSize)) {
            return '0.75rem';
        } else if (/^larger$/i.test(fontSize)) {
            return '1.2rem';
        } else if (/^large$/i.test(fontSize)) {
            return '1.125rem';
        } else if (/^x-large$/i.test(fontSize)) {
            return '1.5rem';
        } else if (/^xx-large$/i.test(fontSize)) {
            return '2rem';
        } else if (/^medium$/i.test(fontSize)) {
            return '1rem';
        } else if (!/pt|px/i.test(fontSize)) {
            return isNaN(size) ? null : size + 'rem';
        }

        var options = {
            useRootSize: !!(_options && _options.useRootSize)
        };

        var s = ENV.win.getComputedStyle(ENV.doc.body.parentNode),
            pxRoot = parseFloat(s.fontSize);
        // 粘贴操作时， pt、px 都按照当前 根字体大小计算 em
        // 其他情况 pt 单位按照 默认字体大小计算 em（因为客户端设置字体的数字都是根据默认大小传递的）
        var ptRoot = options.useRootSize ? defaultFontSize * domUtils.getRootSizeRate() : defaultFontSize;

        if (isNaN(pxRoot) || isNaN(size) || pxRoot == 0) {
            return null;
        }
        if (/pt/i.test(fontSize)) {
            size = size / ptRoot;
        } else {
            size = size / pxRoot;
        }
        return (Math.round((size) * 1000)) / 1000 + 'rem';
    },
    /**
     * 根据 dom 树索引集合 获取 dom
     * @param indexList
     * @returns {*}
     */
    getFromIndexList: function (indexList) {
        if (!indexList || indexList.length === 0) {
            return null;
        }
        var i, j, d, offset;
        d = ENV.doc.body;
        try {
            for (i = 0, j = indexList.length - 1; i < j; i++) {
                d = d.childNodes[indexList[i]];
            }
            offset = indexList[i];
            return {dom: d, offset: offset};
        } catch (e) {
            return null;
        }
    },
    /**
     * 获取 dom 子孙元素中最后一个 叶子节点
     * @param obj
     * @returns {*}
     */
    getLastDeepChild: function (obj) {
        if (!obj) {
            return null;
        }
        var tmp = obj;
        while (tmp && tmp.childNodes && tmp.childNodes.length > 0) {
            tmp = tmp.childNodes[tmp.childNodes.length - 1];
            while (tmp && tmp.nodeType === 1 && /^wiz_/ig.test(tmp.tagName)) {
                tmp = tmp.previousSibling;
            }
            if (tmp) {
                obj = tmp;
            }
        }
        return obj;
    },
    /**
     * 获取 图片数据
     * @param img
     * @returns {*}
     */
    getImageData: function (img) {
        var size = domUtils.getImageSize(img.src);
        // Create an empty canvas element
        var canvas = ENV.doc.createElement("canvas");
        canvas.width = size.width;
        canvas.height = size.height;

        // Copy the image contents to the canvas
        var ctx = canvas.getContext("2d");
        ctx.drawImage(img, 0, 0);

        // Get the data-URL formatted image
        // Firefox supports PNG and JPEG. You could check img.src to
        // guess the original format, but be aware the using "image/jpg"
        // will re-encode the image.
        var dataURL = canvas.toDataURL("image/png");

        return dataURL.replace(/^data:image\/(png|jpg);base64,/, "");
    },
    /**
     * 获取 图片 宽高
     * @param imgSrc
     * @returns {{width: Number, height: Number}}
     */
    getImageSize: function (imgSrc) {
        var newImg = new Image();
        newImg.src = imgSrc;
        var height = newImg.height;
        var width = newImg.width;
        return {width: width, height: height};
    },
    /**
     * 获取 Dom 在当前相邻节点中的 位置（index）
     * @param dom
     * @returns {number}
     */
    getIndex: function (dom) {
        if (!dom || !dom.parentNode) {
            return -1;
        }
        var k = 0, e = dom;
        while ((e = e.previousSibling)) {
            ++k;
        }
        return k;
    },
    /**
     * 获取 dom 在 dom 树内的 索引集合
     * @param dom
     * @returns {Array}
     */
    getIndexList: function (dom) {
        var e = dom, indexList = [];
        while (e && !domUtils.isBody(e)) {
            indexList.splice(0, 0, domUtils.getIndex(e));
            e = e.parentNode;
        }
        return indexList;
    },
    /**
     * 获取 DomA 到 DomB 中包含的所有 叶子节点
     * @param options
     * @returns {{}}
     */
    getListA2B: function (options) {
        var startDom = options.startDom,
            startOffset = options.startOffset,
            endDom = options.endDom,
            endOffset = options.endOffset,
            noSplit = !!options.noSplit,
            isText, changeStart = false, changeEnd = false;

        //修正 start & end 位置
        if (startDom.nodeType === 1 && startOffset > 0 && startOffset < startDom.childNodes.length) {
            startDom = startDom.childNodes[startOffset];
            startOffset = 0;
        }
        if (endDom.nodeType === 1 && endOffset > 0 && endOffset < endDom.childNodes.length) {
            endDom = endDom.childNodes[endOffset];
            endOffset = 0;
        }
        //如果起始点 和终止点位置不一样， 且 endOffset == 0，则找到 endOom 前一个叶子节点
        if (startDom !== endDom && endOffset === 0) {
            endDom = domUtils.getPreviousNode(endDom, false, startDom);
            if (!endDom) {
                endDom = startDom;
            }

            if (domUtils.isSelfClosingTag(endDom)) {
                //如果 修正后的 endDom 为 自闭合标签， 需要特殊处理
                endOffset = 1;
            } else {
                endOffset = domUtils.getEndOffset(endDom);
            }
        }

        // get dom which is start and end
        if (startDom === endDom && startOffset !== endOffset) {
            isText = (startDom.nodeType === 3);
            if (isText && !startDom.parentNode.getAttribute(CONST.ATTR.SPAN_DELETE) &&
                !noSplit) {
                startDom = domUtils.splitRangeText(startDom, startOffset, endOffset, false);
                endDom = startDom;
                changeStart = true;
                changeEnd = true;
            } else if (startDom.nodeType === 1 &&
                startDom.childNodes.length > 0 && !domUtils.isSelfClosingTag(startDom)) {
                startDom = startDom.childNodes[startOffset];
                endDom = endDom.childNodes[endOffset - 1];
                changeStart = true;
                changeEnd = true;
            }
        } else if (startDom !== endDom) {
            if (startDom.nodeType === 3 && !startDom.parentNode.getAttribute(CONST.ATTR.SPAN_DELETE) &&
                !noSplit) {
                startDom = domUtils.splitRangeText(startDom, startOffset, null, false);
                changeStart = true;
            } else if (startDom.nodeType === 1 && startDom.childNodes.length > 0 &&
                startOffset < startDom.childNodes.length) {
                startDom = startDom.childNodes[startOffset];
                changeStart = true;
            }
            if (endDom.nodeType === 3 && endOffset > 0 && !endDom.parentNode.getAttribute(CONST.ATTR.SPAN_DELETE) &&
                !noSplit) {
                endDom = domUtils.splitRangeText(endDom, 0, endOffset, true);
                changeEnd = true;
            } else if (!domUtils.isSelfClosingTag(endDom) && endDom.nodeType === 1 && endOffset > 0) {
                endDom = domUtils.getLastDeepChild(endDom.childNodes[endOffset - 1]);
                changeEnd = true;
            }
        }
        if (changeStart) {
            startOffset = 0;
        }
        if (changeEnd) {
            endOffset = domUtils.getEndOffset(endDom);
        }

        //make the array
        var curDom = startDom, result = [];
        if (startOffset === startDom.length) {
            curDom = domUtils.getNextNode(curDom, false, endDom);
        }

        while (curDom && !(startDom === endDom && startOffset === endOffset)) {
            if (curDom === endDom || curDom === endDom.parentNode) {
                addDomForGetDomList(result, endDom);
                break;
            } else if (domUtils.isBody(curDom)) {
                addDomForGetDomList(result, curDom);
                break;
            } else {
                addDomForGetDomList(result, curDom);
            }
            curDom = domUtils.getNextNode(curDom, false, endDom);
        }

        // startDom 和 endDom 在 clearChild 操作中可能会被删除，所以必须要记住边缘 Dom 范围
        var startDomBak = domUtils.getPreviousNode(result[0], false, null),
            endDomBak = domUtils.getNextNode(result[result.length - 1], false, null);
        if (startDomBak && startDomBak.nodeType === 1 && startDomBak.firstChild) {
            startDomBak = startDomBak.firstChild;
        }
        if (endDomBak && endDomBak.nodeType === 1 && endDomBak.lastChild) {
            endDomBak = endDomBak.lastChild;
        }
        var startOffsetBak = domUtils.getEndOffset(startDomBak),
            endOffsetBak = 0;

        return {
            list: result,
            startDom: startDom,
            startOffset: startOffset,
            endDom: endDom,
            endOffset: endOffset,
            startDomBak: startDomBak,
            startOffsetBak: startOffsetBak,
            endDomBak: endDomBak,
            endOffsetBak: endOffsetBak
        };

        function addDomForGetDomList (main, sub) {
            main.push(sub);
        }
    },
    /**
     * 在 Index 范围内根据指定 tagName 获取 DOM 集合
     * @param tagName
     * @param startIndex  //1.1.5
     * @param endIndex    //1.4.2
     * @returns {Array}
     */
    getListFromTagAndIndex: function (tagName, container, startIndex, endIndex) {
        var result = [];
        var tagList = container.querySelectorAll(tagName);
        var i, tagIndex, tag;
        for (i = 0; i < tagList.length; i++) {
            tag = tagList[i];
            tagIndex = domUtils.getIndexList(tag).join('.');
            if (utils.compareVersion(tagIndex, startIndex) > -1 &&
                utils.compareVersion(tagIndex, endIndex) < 1) {
                result.push(tag);
            }
        }
        return result;
    },
    /**
     * 获取 DOM 的 下一个 可编辑的叶子节点
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getNextNodeCanEdit: function (dom, onlyElement, endDom) {
        dom = domUtils.getNextNode(dom, onlyElement, endDom);
        while (dom && !domUtils.canEdit(dom)) {
            dom = domUtils.getNextNode(dom, onlyElement, endDom);
        }
        return dom;
    },
    /**
     * 获取 DOM 的下一个叶子节点（包括不相邻的情况），到达指定的 endDom 为止（如果为空则忽略）
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getNextNode: function (dom, onlyElement, endDom) {
        var originalDom = dom;
        if (!dom || dom == endDom) {
            return null;
        }
        onlyElement = !!onlyElement;

        function next (d) {
            if (!d) {
                return null;
            }
            return onlyElement ? d.nextElementSibling : d.nextSibling;
        }

        function first (d) {
            if (!d) {
                return null;
            }
            return onlyElement ? d.firstElementChild : d.firstChild;
        }

        if (!next(dom) && !dom.parentNode) {
            return null;
        } else if (!next(dom)) {
            //if hasn't nextSibling,so find its parent's nextSibling
            while (dom.parentNode) {
                dom = dom.parentNode;
                //必须首先判断 是否 body
                if (domUtils.isBody(dom)) {
                    dom = null;
                    break;
                }
                if (dom === endDom) {
                    break;
                }
                if (next(dom)) {
                    dom = next(dom);
                    break;
                }
            }
        } else {
            dom = next(dom);
        }

        if (dom === endDom) {
            return dom;
        }

        //if next node has child nodes, so find the first child node.
        var tmpD;
        tmpD = first(dom);
        if (!!dom && tmpD) {
            while (tmpD) {
                dom = tmpD;
                if (dom === endDom) {
                    break;
                }
                tmpD = first(tmpD);
            }
        }
        if (dom === originalDom) {
            return null;
        }
        return dom;
    },
    /**
     * 获取 offset 结果
     * @param dom
     * @returns {{top: number, left: number}}
     */
    getOffset: function (dom) {
        var offset = {top: 0, left: 0};
        if (dom.offsetParent) {
            while (dom.offsetParent) {
                offset.top += dom.offsetTop;
                offset.left += dom.offsetLeft;
                dom = dom.offsetParent;
            }
        } else {
            offset.left += dom.offsetLeft;
            offset.top += dom.offsetTop;
        }
        return offset;
    },
    /**
     * 获取 页面滚动条位置
     * @returns {{}}
     */
    getPageScroll: function () {
        var scroll = {};
        if (typeof ENV.win.pageYOffset != 'undefined') {
            scroll.left = ENV.win.pageXOffset;
            scroll.top = ENV.win.pageYOffset;
        } else if (typeof ENV.doc.compatMode != 'undefined' && ENV.doc.compatMode != 'BackCompat') {
            scroll.left = ENV.doc.documentElement.scrollLeft;
            scroll.top = ENV.doc.documentElement.scrollTop;
        } else if (typeof ENV.doc.body != 'undefined') {
            scroll.left = ENV.doc.body.scrollLeft;
            scroll.top = ENV.doc.body.scrollTop;
        }
        return scroll;
    },
    /**
     * 根据 filterFn 函数设置的 自定义规则 查找 Dom 的父节点
     * @param node
     * @param filterFn
     * @param includeSelf
     * @returns {*}
     */
    getParentByFilter: function (node, filterFn, includeSelf) {
        if (node && !domUtils.isBody(node)) {
            node = includeSelf ? node : node.parentNode;
            while (node) {
                if (!filterFn || filterFn(node)) {
                    return node;
                }
                if (domUtils.isBody(node)) {
                    return null;
                }
                node = node.parentNode;
            }
        }
        return null;
    },
    /**
     * 根据 ClassName 查找 Dom 的父节点
     * @param node
     * @param className
     * @param includeSelf
     * @returns {*}
     */
    getParentByClass: function (node, className, includeSelf) {
        return domUtils.getParentByFilter(node, function (dom) {
            return domUtils.hasClass(dom, className);
        }, includeSelf);
    },
    /**
     * 根据 Tag 名称查找 Dom 的父节点
     * @param node
     * @param tagNames
     * @param includeSelf
     * @param excludeFn
     * @returns {*}
     */
    getParentByTagName: function (node, tagNames, includeSelf, excludeFn) {
        if (!node) {
            return null;
        }
        tagNames = utils.listToMap(utils.isArray(tagNames) ? tagNames : [tagNames]);
        return domUtils.getParentByFilter(node, function (node) {
            return tagNames[node.tagName] && !(excludeFn && excludeFn(node));
        }, includeSelf);
    },
    /**
     * 获取多个 dom 共同的父节点
     * @param domList
     */
    getParentRoot: function (domList) {
        if (!domList || domList.length === 0) {
            return null;
        }
        var i, j, tmpIdx, pNode, parentList = [];
        pNode = domList[0].nodeType == 1 ? domList[0] : domList[0].parentNode;
        while (pNode && !domUtils.isBody(pNode)) {
            parentList.push(pNode);
            pNode = pNode.parentNode;
        }
        for (i = 1, j = domList.length; i < j; i++) {
            pNode = domList[i];
            while (pNode) {
                if (domUtils.isBody(pNode)) {
                    return ENV.doc.body;
                }
                tmpIdx = parentList.indexOf(pNode);
                if (tmpIdx > -1) {
                    parentList.splice(0, tmpIdx);
                    break;
                }
                pNode = pNode.parentNode;
            }
        }
        if (parentList.length === 0) {
            return ENV.doc.body;
        } else {
            return parentList[0];
        }
    },
    getParentList: function (obj) {
        var list = [];
        var p = obj.parentNode;
        while (!!p && p !== ENV.doc.body) {
            list.splice(0, 0, p);
            p = p.parentNode;
        }
        return list;
    },
    /**
     * 获取 DOM 的 坐标 & 大小
     * @param obj
     * @returns {*}
     */
    getPosition: function (obj) {
        if (!obj) {
            return null;
        }
        return {
            top: obj.offsetTop,
            left: obj.offsetLeft,
            height: obj.offsetHeight,
            width: obj.offsetWidth
        };
    },
    /**
     * 获取 DOM 的前一个叶子节点（包括不相邻的情况），到达指定的 startDom 为止（如果为空则忽略）
     * @param dom
     * @param onlyElement
     * @param startDom
     * @returns {*}
     */
    getPreviousNode: function (dom, onlyElement, startDom) {
        var originalDom = dom;
        if (!dom || dom == startDom) {
            return null;
        }
        onlyElement = !!onlyElement;

        function prev (d) {
            return onlyElement ? d.previousElementSibling : d.previousSibling;
        }

        function last (d) {
            return onlyElement ? d.lastElementChild : d.lastChild;
        }

        if (!prev(dom)) {
            //if hasn't previousSibling,so find its parent's previousSibling
            while (dom.parentNode) {
                dom = dom.parentNode;
                //必须首先判断 是否 body
                if (domUtils.isBody(dom)) {
                    dom = null;
                    break;
                }
                if (dom == startDom) {
                    break;
                }
                if (prev(dom)) {
                    dom = prev(dom);
                    break;
                }
            }
        } else {
            dom = prev(dom);
        }

        if (!dom) {
            return null;
        }
        //对于查找前一个dom节点的算法 与 查找 下一个dom的算法略有不同
        //如果 dom 与 startDom 相同， 但 dom 有子元素的时候， 不能直接返回 dom
        if (dom === startDom &&
            ((dom.nodeType === 3) ||
                dom.nodeType === 1 && dom.childNodes.length === 0)) {
            return dom;
        }

        //if previous node has child nodes, so find the last child node.
        var tmpD;
        tmpD = last(dom);
        if (!!dom && tmpD) {
            while (tmpD) {
                dom = tmpD;
                if (dom === startDom &&
                    ((dom.nodeType === 3) ||
                        dom.nodeType === 1 && dom.childNodes.length === 0)) {
                    break;
                }
                tmpD = last(tmpD);
            }
        }

        if (dom === originalDom) {
            // 如果最终找到的 last child node 还是原始的 dom ，那么说明没有 prev
            return null;
        }

        return dom;
    },
    /**
     * 获取 DOM 的 下一个 可编辑的叶子节点
     * @param dom
     * @param onlyElement
     * @param endDom
     * @returns {*}
     */
    getPreviousNodeCanEdit: function (dom, onlyElement, endDom) {
        dom = domUtils.getPreviousNode(dom, onlyElement, endDom);
        while (dom && !domUtils.canEdit(dom)) {
            dom = domUtils.getPreviousNode(dom, onlyElement, endDom);
        }
        return dom;
    },
    /**
     * 给 dom 内添加 Tab 时 获取 4 个 ' '
     */
    getTab: function () {
        var x = ENV.doc.createElement('span');
        x.innerHTML = '&nbsp;&nbsp;&nbsp;&nbsp;';
        return (x.childNodes[0]);
    },
    /**
     * 获取 td,th 单元格 在 table 中的 行列坐标
     * @param td
     */
    getTdIndex: function (td) {
        return {
            x: td.cellIndex,
            y: td.parentNode.rowIndex,
            maxX: td.parentNode.cells.length,
            maxY: td.parentNode.parentNode.rows.length
        };
    },
    /**
     * 获取窗口可视区域大小
     * @returns {{width: number, height: number}}
     */
    getWindowSize: function () {
        return {
            width: ENV.doc.documentElement.clientWidth,
            height: ENV.doc.documentElement.clientHeight
        };
    },
    /**
     * 根据 dom 获取其 修订的父节点， 如果不是 修订内容，则返回空
     * @param dom
     * @returns {*}
     */
    getWizAmendParent: function (dom) {
        return domUtils.getParentByFilter(dom, function (node) {
            //node.childNodes.length == 0 时，键盘敲入的字符加在 span 外面
            return ( node && node.nodeType === 1 && (node.getAttribute(CONST.ATTR.SPAN_INSERT) ||
                node.getAttribute(CONST.ATTR.SPAN_DELETE)) );
        }, true);
    },
    /**
     * 判断 dom 是否含有 某个 class name
     * @param obj
     * @param className
     * @returns {boolean}
     */
    hasClass: function (obj, className) {
        if (obj && obj.nodeType === 1) {
            return ((' ' + obj.className + ' ').indexOf(' ' + className + ' ') > -1);
        }
        return false;
    },
    after: function (newDom, target) {
        domUtils.before(newDom, target, true);
    },
    /**
     * wiz 编辑器使用的 插入 dom 方法（isAfter 默认为 false，即将 dom 插入到 target 前面）
     * @param newDom
     * @param target
     * @param isAfter
     */
    before: function (newDom, target, isAfter) {
        isAfter = !!isAfter;
        if (!target || !newDom) {
            return;
        }
        var isBody = target === ENV.doc.body,
            parent = isBody ? target : target.parentNode,
            nextDom = isBody ? (isAfter ? null : ENV.doc.body.childNodes[0]) : (isAfter ? target.nextSibling : target);
        var i, d, last;
        if (!utils.isArray(newDom)) {
            parent.insertBefore(newDom, nextDom);
        } else {
            last = nextDom;
            for (i = newDom.length - 1; i >= 0; i--) {
                d = newDom[i];
                parent.insertBefore(d, last);
                last = d;
            }
        }
    },
    /**
     * 判断 dom 是否为 块级元素
     * @param dom
     * @returns {boolean}
     */
    isBlock: function (dom) {
        if (!dom) {
            return false;
        }
        // if (dom.nodeType == 9 || dom.nodeType == 11) {
        //     return true;
        // }

        if (domUtils.isWizDom(dom)) {
            // 对于 Wiz 专用 tag 全部认为是 block dom
            return true;
        }
        var displayValue = domUtils.getComputedStyle(dom, 'display', false);
        return !!displayValue && !/^(inline|inline\-block|inline\-table|none)$/i.test(displayValue);
    },
    isWizDom: function (dom) {
        return (dom.nodeType === 1 && /^wiz_/i.test(dom.tagName));
    },
    /**
     * 判断 dom 是否为 document.body
     * @param dom
     * @returns {*|boolean|boolean}
     */
    isBody: function (dom) {
        return dom && dom == ENV.doc.body;
    },
    /**
     * 判断 dom 是否为空（里面仅有 br 时 也被认为空）
     * @param dom
     * @returns {*}
     */
    isEmptyDom: function (dom) {
        if (!dom) {
            return false;
        }
        var i, j, v;
        if (dom.nodeType === 3) {
            v = dom.nodeValue;
            return utils.isEmpty(v);
        }

        if (dom.nodeType !== 1) {
            return true;
        }

        if (dom.childNodes.length === 0) {
            return domUtils.isTag(dom, 'br') || !domUtils.isSelfClosingTag(dom);
        }

        for (i = 0, j = dom.childNodes.length; i < j; i++) {
            if (!domUtils.isEmptyDom(dom.childNodes[i])) {
                return false;
            }
        }
        return true;
    },
    /**
     * 判断 dom 内容是否为 填充的特殊字符
     * @param node
     * @param isInStart
     * @returns {boolean}
     */
    isFillChar: function (node, isInStart) {
        return node.nodeType === 3 && !node.nodeValue.replace(new RegExp((isInStart ? '^' : '') + CONST.FILL_CHAR), '').length;
    },
    /**
     * 判断 dom 是否为 行级元素
     * @param dom
     * @returns {boolean}
     */
    isInlineDom: function (dom) {
        return !domUtils.isBlock(dom);
    },
    /**
     * 判断 dom 是否为 自闭和标签 （主要用于清理冗余 dom 使用，避免 dom 被删除）
     * @param node
     * @returns {boolean}
     */
    isSelfClosingTag: function (node) {
        var selfLib = /^(area|base|br|col|command|embed|hr|img|input|keygen|link|meta|param|source|track|wbr)$/i;
        return (node && node.nodeType === 1 && selfLib.test(node.tagName));
    },
    /**
     * 判断两个 span 属性（style & attribute）是否相同（属性相同且相邻的两个 span 才可以合并）
     * @param n
     * @param m
     * @returns {boolean}
     */
    isSameSpan: function (n, m) {
        return !!n && !!m && n.nodeType == 1 && m.nodeType == 1 &&
            domUtils.isTag(n, 'span') && n.tagName == m.tagName &&
            n.getAttribute(CONST.ATTR.SPAN) == CONST.ATTR.SPAN &&
            domUtils.isSameStyle(n, m) && domUtils.isSameAttr(n, m);
    },
    /**
     * 判断两个 dom 的 attribute 是否相同
     * @param n
     * @param m
     * @returns {boolean}
     */
    isSameAttr: function (n, m) {
        var attrA = n.attributes,
            attrB = m.attributes;
        if (attrA.length != attrB.length) {
            return false;
        }
        var i, j, a;
        for (i = 0, j = attrA.length; i < j; i++) {
            a = attrA[i];
            if (a.name == 'style') {
                continue;
            }
            if (a.name === CONST.ATTR.SPAN_TIMESTAMP) {
                if (!utils.isSameAmendTime(a.value, attrB[a.name].value)) {
                    return false;
                }
                continue;
            } else if (!attrB[a.name] || attrB[a.name].value != a.value) {
                return false;
            }
        }
        return true;
    },
    /**
     * 判断 dom 的 style （inline）是否相同
     * @param n
     * @param m
     */
    isSameStyle: function (n, m) {
        var styleA = {};
        var styleB = {};
        domUtils.appendStyle(n.getAttribute('style'), styleA);
        domUtils.appendStyle(m.getAttribute('style'), styleB);
        var k;
        for (k in styleA) {
            if (styleA.hasOwnProperty(k)) {
                if (styleB[k] !== styleA[k]) {
                    return false;
                }
                delete styleA[k];
                delete styleB[k];
            }
        }
        for (k in styleB) {
            if (styleB.hasOwnProperty(k)) {
                return false;
            }
        }
        return true;
    },
    /**
     * 判断 dom 是否为指定的 tagName
     * @param dom
     * @param tagNames
     * @returns {boolean}
     */
    isTag: function (dom, tagNames) {
        if (!utils.isArray(tagNames)) {
            tagNames = [tagNames];
        }
        if (!dom || dom.nodeType !== 1) {
            return false;
        }
        var i, j, tag = dom.tagName.toLowerCase();
        for (i = 0, j = tagNames.length; i < j; i++) {
            if (tag === tagNames[i].toLowerCase()) {
                return true;
            }
        }
        return false;
    },
    /**
     * 判断 TextNode 内容是否为 非空 有效
     * @param node
     * @returns {boolean}
     */
    isUsableTextNode: function (node) {
        return node.nodeType == 3 && (!utils.isEmpty(node.nodeValue));
    },
    /**
     * 判断 dom 是否为 wiz 编辑器 的 span
     * @param dom
     * @returns {boolean}
     */
    isWizSpan: function (dom) {
        return !!dom && !!dom.getAttribute(CONST.ATTR.SPAN);
    },
    /**
     * 把 domA 合并到 domB （仅合并 attribute 和 style）
     * @param objA
     * @param objB
     * @param isOverlay
     */
    mergeAtoB: function (objA, objB, isOverlay) {
        domUtils.mergeStyleAToB(objA, objB, isOverlay);
        domUtils.mergeAttrAtoB(objA, objB, isOverlay);
    },
    /**
     * 把 domA 的属性（attribute） 合并到 domB
     * @param objA
     * @param objB
     * @param isOverlay
     */
    mergeAttrAtoB: function (objA, objB, isOverlay) {
        if (objA.nodeType != 1 || objB.nodeType != 1) {
            return;
        }
        var attrA = objA.attributes,
            attrB = objB.attributes,
            i, j, a;
        for (i = 0, j = attrA.length; i < j; i++) {
            a = attrA[i];
            if (a.name == 'style') {
                continue;
            }
            if (attrB[a.name] && !isOverlay) {
                continue;
            }
            objB.setAttribute(a.name, a.value);
        }
    },
    /**
     * 把 domA 的样式（style） 合并到 domB
     * @param objA
     * @param objB
     * @param isOverlay
     */
    mergeStyleAToB: function (objA, objB, isOverlay) {
        if (objA.nodeType != 1 || objB.nodeType != 1) {
            return;
        }
        var sA = objA.getAttribute('style'),
            sB = objB.getAttribute('style') || '';
        if (!sA) {
            return;
        }
        var styleObj = {};
        if (!!isOverlay) {
            domUtils.appendStyle(sB, styleObj);
            domUtils.appendStyle(sA, styleObj);
        } else {
            domUtils.appendStyle(sA, styleObj);
            domUtils.appendStyle(sB, styleObj);
        }

        var result = [];
        for (var k in styleObj) {
            if (styleObj.hasOwnProperty(k)) {
                result.push(k + ':' + styleObj[k]);
            }
        }
        objB.setAttribute('style', result.join(';'));
    },
    /**
     * 针对 body、td、th 内的 textNode 封装 div
     * @param dom
     * @returns {*}
     */
    packageByDiv: function (dom) {
        var p, packDiv;
        p = dom;
        while (p) {
            if (domUtils.isBlock(p)) {
                packDiv = p;
                p = null;
                break;
            }
            if (domUtils.isTag(p.parentNode, ['body', 'td', 'th'])) {
                break;
            }

            p = p.parentNode;
        }

        if (packDiv) {
            return packDiv;
        }

        var start = p,
            end = p, tmp;
        while (start) {
            tmp = start.previousSibling;
            if (!tmp || domUtils.isBlock(tmp)) {
                break;
            }
            start = tmp;
        }
        while (end) {
            tmp = end.nextSibling;
            if (!tmp || domUtils.isBlock(tmp)) {
                break;
            }
            end = tmp;
        }

        packDiv = ENV.doc.createElement('div');
        domUtils.before(packDiv, start);
        var tmp = start, next, isLast;
        do {
            next = tmp.nextSibling;
            packDiv.appendChild(tmp);
            isLast = tmp === end;
            tmp = next;
        } while (!isLast);

        return packDiv;
    },
    /**
     * 将 dom 剥壳
     * @param dom
     */
    peelDom: function (dom, checkFun) {
        if (!dom || dom.nodeType === 3) {
            return;
        }
        var result = {
            start: null,
            end: null
        };
        var childNodes, child, i;
        childNodes = dom.childNodes;
        for (i = childNodes.length - 1; i >= 0; i--) {
            child = childNodes[i];
            if (!checkFun || checkFun(child)) {
                domUtils.after(child, dom);
                if (!result.start) {
                    result.start = child;
                    result.end = child;
                } else {
                    result.end = child;
                }
            }
        }
        domUtils.remove(dom);
        return result;
    },
    /**
     * 从 dom 中清除指定 tag 标签，但保留 innerHTML
     * @param tag
     * @returns {void|*|XML|string}
     */
    peelTag: function (tagName) {
        var list = ENV.doc.querySelectorAll(tagName),
            i, tag;
        for (i = list.length - 1; i >= 0; i--) {
            tag = list[i];
            while (tag.firstChild) {
                domUtils.before(tag.firstChild, tag);
            }
            domUtils.remove(tag);
        }
    },
    /**
     * 从 html 源码中清除指定 tag 标签，但保留 innerHTML
     * @param html
     * @param tag
     * @returns {void|*|XML|string}
     */
    peelTagFromHtml: function (html, tag) {
        var reg = new RegExp('<' + tag + '( [^>]*)?>|<\/' + tag + '>', 'ig');
        return html.replace(reg, '');
    },
    replaceTagName: function (dom, tagName) {
        if (domUtils.isTag(dom, tagName)) {
            return;
        }
        var isTodo = domUtils.hasClass(dom, CONST.CLASS.TODO_LAYER);
        var tag = ENV.doc.createElement(tagName);
        domUtils.after(tag, dom);
        while (dom.firstChild) {
            tag.appendChild(dom.firstChild);
        }
        if (isTodo) {
            if (dom.className) {
                tag.className = dom.className;
            }
            if (dom.id) {
                tag.id = dom.id;
            }
        }
        domUtils.remove(dom);
    },
    /**
     * 移除 class name
     * @param domList
     * @param className
     */
    removeClass: function (domList, className) {
        if (!domList) {
            return;
        }
        if (!!domList.nodeType) {
            domList = [domList];
        }
        if (!utils.isArray(className)) {
            className = [className];
        }
        var i, j, dom, css;
        for (i = domList.length - 1; i >= 0; i--) {
            dom = domList[i];
            if (dom.nodeType === 1) {
                dom.className = (" " + dom.className + " ");
                for (j = className.length; j >= 0; j--) {
                    css = className[j];
                    dom.className = dom.className.replace(' ' + css + ' ', ' ');
                }
                dom.className = dom.className.trim();
            }
        }
    },
    remove: function (dom) {
        if (!dom || !dom.parentNode) {
            return;
        }
        dom.parentNode.removeChild(dom);
    },
    /**
     * 从 Dom 中清除指定 name 的 tag
     * @param name
     */
    removeByName: function (name) {
        var s = ENV.doc.getElementsByName(name);
        var i, dom;
        for (i = s.length - 1; i >= 0; i--) {
            dom = s[i];
            domUtils.remove(dom);
        }
    },
    /**
     * 从 Dom 中清除指定 的 tag
     * @param tag
     */
    removeByTag: function (tag) {
        var s = ENV.doc.getElementsByTagName(tag);
        var i, dom;
        for (i = s.length - 1; i >= 0; i--) {
            dom = s[i];
            domUtils.remove(dom);
        }
    },
    /**
     * 从 html 源码中清除指定的 tag （注意，一定要保证该 tag 内不存在嵌套同样 tag 的情况）
     * @param html
     * @param tag
     * @returns {string}
     */
    removeByTagFromHtml: function (html, tag) {
        var reg = new RegExp('<' + tag + '( [^>]*)?>((?!<\/' + tag + '>).|\r|\n)*?<\/' + tag + '>', 'ig');
        return html.replace(reg, '');
    },
    /**
     * 从 html 源码中清除指定 id 的 style
     * 因为使用正则，不可能直接将 有嵌套 div 的div html 代码删除，所以此函数只能针对 style 等不会包含 html 代码的 tag 进行操作
     * @param html
     * @param idList
     * @returns {string}
     */
    removeStyleByIdFromHtml: function (html, idList) {
        var id = '(' + idList.join('|') + ')';
        var regStyle = new RegExp('<style( ([^<>])+[ ]+|[ ]+)id *= *([\'"])' + id + '\\3[^<>]*>[^<]*<\/style>', 'ig');
        var regLink = new RegExp('<link( ([^<>])+[ ]+|[ ]+)id *= *([\'"])' + id + '\\3[^<>]*>', 'ig');
        return html.replace(regStyle, '').replace(regLink, '');
    },
    /**
     * 从 html 源码中清除指定 name 的 style
     * 因为使用正则，不可能直接将 有嵌套 div 的div html 代码删除，所以此函数只能针对 style 等不会包含 html 代码的 tag 进行操作
     * @param html
     * @param nameList
     * @returns {string}
     */
    removeStyleByNameFromHtml: function (html, nameList) {
        var name = '(' + nameList.join('|') + ')';
        var regStyle = new RegExp('<style( ([^<>])+[ ]+|[ ]+)name *= *([\'"])' + name + '\\3[^<>]*>[^<]*<\/style>', 'ig');
        var regLink = new RegExp('<link( ([^<>])+[ ]+|[ ]+)name *= *([\'"])' + name + '\\3[^<>]*>', 'ig');
        return html.replace(regStyle, '').replace(regLink, '');
    },
    removeViewportFromHtml: function (html, tag) {
        //检查 head 内存在 viewport 才删除，避免将 code 内的 viewport 删除
        if (ENV.doc.querySelector('head meta[name=viewport]')) {
            var reg = new RegExp('<meta( ([^<>])+[ ]+|[ ]+)name *= *([\'"])viewport\\3[^<>]*>', 'i');
            return html.replace(reg, '');
        }
        return html;
    },
    /**
     * 从 dom 集合中删除符合特殊规则的 dom
     * @param domList
     * @param filter
     * @returns {Array} 返回被删除的集合列表
     */
    removeListFilter: function (domList, filter) {
        var removeList = [], i, dom;

        if (!domList || !filter) {
            return removeList;
        }

        for (i = domList.length - 1; i >= 0; i--) {
            dom = domList[i];
            if (filter(dom)) {
                removeList.unshift(domList.splice(i, 1)[0]);
            }
        }
        return removeList;
    },
    /**
     * 根据 查询表达式 查找 dom，并放到 list 集合内
     * @param dom
     * @param expStr
     * @param list
     */
    search: function (dom, expStr, list) {
        //TODO 兼容问题
        var tmpList = dom.querySelectorAll(expStr),
            i, j, d;
        list = list ? list : [];
        for (i = 0, j = tmpList.length; i < j; i++) {
            d = tmpList[i];
            list.push(d);
        }
    },
    /**
     * 设置区域可编辑
     * @param content
     * @param enable
     */
    setContenteditable: function (content, enable) {
        if (!content && ENV.win.WizTemplate) {
            ENV.win.WizTemplate.setContenteditable(enable);
        } else {
            if (!content) {
                content = ENV.doc.body;
            }
            content.setAttribute('contenteditable', enable ? 'true' : 'false');
        }
    },
    /**
     * 自动布局（根据 target 的位置 以及 屏幕大小，设置 layerObj 的坐标，保证在可视区域内显示）
     * @param options
     * {layerObj, target, layout, fixed, noSpace, reverse}
     */
    setLayout: function (options) {
        var layerObj = options.layerObj,
            target = options.target,
            layout = options.layout,
            fixed = !!options.fixed,
            noSpace = !!options.noSpace,
            reverse = !!options.reverse;

        var confirmPos = domUtils.getPosition(layerObj),
            targetPos = target.nodeType ? domUtils.getPosition(target) : target,
            scrollPos = domUtils.getPageScroll(),
            winWidth = ENV.doc.documentElement.clientWidth,
            winHeight = ENV.doc.documentElement.clientHeight,
            bodyTop = window.getComputedStyle ? ENV.win.getComputedStyle(ENV.doc.body, null)['margin-top'] : 0,
            left = '50%', top = '30%',
            mTop = 0,
            mLeft = -(confirmPos.width) / 2,
            minWidth, maxWidth,
            minHeight, maxHeight;


        //iphone 客户端 编辑时 window 窗口顶端有其他 window 遮罩， 所以必须要计算 body 的 margin-top
        if (!!bodyTop) {
            bodyTop = parseInt(bodyTop);
            if (isNaN(bodyTop)) {
                bodyTop = 0;
            }
        }

        if (fixed) {
            minWidth = 0;
            maxWidth = winWidth - 5; //右侧需要保留一些空间，避免有时候超出
            minHeight = 0 + bodyTop;
            maxHeight = winHeight;
        } else {
            minWidth = 0 + scrollPos.left;
            maxWidth = winWidth + scrollPos.left - 5; //右侧需要保留一些空间，避免有时候超出
            minHeight = 0 + (scrollPos.top <= bodyTop ? 0 : Math.abs(scrollPos.top - bodyTop)) + bodyTop;
            maxHeight = winHeight + scrollPos.top;
        }

        if (targetPos && layout) {
            mTop = 0;
            mLeft = 0;
            if (layout == CONST.TYPE.POS.upLeft || layout == CONST.TYPE.POS.upRight) {
                top = targetPos.top - confirmPos.height - (noSpace ? 0 : CONST.AMEND.INFO_SPACE);
            } else if (layout == CONST.TYPE.POS.downLeft || layout == CONST.TYPE.POS.downRight) {
                top = targetPos.top + targetPos.height + (noSpace ? 0 : CONST.AMEND.INFO_SPACE);
            } else if (layout == CONST.TYPE.POS.leftUp || layout == CONST.TYPE.POS.leftDown) {
                left = targetPos.left - confirmPos.width - (noSpace ? 0 : CONST.AMEND.INFO_SPACE);
            } else if (layout == CONST.TYPE.POS.rightUp || layout == CONST.TYPE.POS.rightDown) {
                left = targetPos.left + targetPos.width + (noSpace ? 0 : CONST.AMEND.INFO_SPACE);
            }

            if (layout == CONST.TYPE.POS.upLeft || layout == CONST.TYPE.POS.downLeft) {
                left = targetPos.left;
                if (fixed) {
                    left -= scrollPos.left;
                }
            } else if (layout == CONST.TYPE.POS.upRight || layout == CONST.TYPE.POS.downRight) {
                left = targetPos.left + targetPos.width - confirmPos.width;
                if (fixed) {
                    left -= scrollPos.left;
                }
            } else if (layout == CONST.TYPE.POS.leftUp || layout == CONST.TYPE.POS.rightUp) {
                top = targetPos.top;
                if (fixed) {
                    top -= scrollPos.top;
                }

            } else if (layout == CONST.TYPE.POS.leftDown || layout == CONST.TYPE.POS.rightDown) {
                top = targetPos.top + targetPos.height - confirmPos.height;
                if (fixed) {
                    top -= scrollPos.top;
                }
            }

            if (left + confirmPos.width > maxWidth) {
                left = maxWidth - confirmPos.width;
            }
            if (left < minWidth) {
                left = minWidth;
            }
            if (top + confirmPos.height > maxHeight) {
                top = maxHeight - confirmPos.height;
            }
            if (reverse && top < minHeight) {
                top = targetPos.top + targetPos.height;
            }
            if (top < minHeight || top + confirmPos.height > maxHeight) {
                top = minHeight;
            }

        }
        domUtils.css(layerObj, {
            left: left + 'px',
            top: top + 'px',
            'margin-top': mTop + 'px',
            'margin-left': mLeft + 'px'
        });
    },
    /**
     * 设置 textarea 的值
     * @param textarea
     * @param text
     */
    setTextarea: function (textarea, text) {
        var textType = (textarea.textContent === undefined ? 'innerText' : 'textContent');
        if (text) {
            textarea[textType] = utils.replaceSpecialChar(text);
            return;
        }
        if (textarea.value !== textarea[textType]) {
            textarea[textType] = textarea.value;
        }
    },

    /**
     * 根据 光标选择范围 拆分 textNode
     * splitRangeText 不能返回 TextNode，所以在 wizSpan 内要把 TextNode 独立分割出来，然后返回其 parentNode
     * @param node
     * @param start
     * @param end
     * @param isLast
     * @returns {*}
     */
    splitRangeText: function (node, start, end, isLast) {
        if (!domUtils.isUsableTextNode(node)) {
            return node;
        }
        var p, s, t, v = node.nodeValue;
        p = node.parentNode;
//            var isWizSpan = domUtils.isWizSpan(p);
        s = domUtils.createSpan();

        if (!start && !end || (start === 0 && end === node.nodeValue.length)) {
            //the range is all text in this node
            // td,th 必须特殊处理，否则会导致 td 被添加 修订样式
            if (p.childNodes.length > 1 || domUtils.isTag(p, ['td', 'th'])) {
                if (isLast) {
                    p.insertBefore(s, node);
                } else {
                    p.insertBefore(s, node.nextSibling);
                }
                s.appendChild(node);
            } else {
                //if textNode is the only child node, return its parent node.
                s = p;
            }
        } else if (start === 0) {
            //the range is [0, n] (n<length)
            s.textContent = v.substring(start, end);
            p.insertBefore(s, node);
            node.nodeValue = v.substring(end);

        } else if (!end || end === node.nodeValue.length) {
            s.textContent = v.substring(start);
            p.insertBefore(s, node.nextSibling);
            node.nodeValue = v.substring(0, start);
        } else {
            //the range is [m, n] (m>0 && n<length)
            t = ENV.doc.createTextNode(v.substring(end));
            s.textContent = v.substring(start, end);
            p.insertBefore(s, node.nextSibling);
            p.insertBefore(t, s.nextSibling);
            //必须要先添加文字，最后删除多余文字，否则，如果先删除后边文字，会导致滚动条跳动
            node.nodeValue = v.substring(0, start);
        }
        return s;
    }
};

module.exports = domUtils;

},{"./../common/const":18,"./../common/env":20,"./../common/utils":24}],29:[function(require,module,exports){
/**
 * DOM 操作工具包（扩展类库）
 */

var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils'),
    domUtils = require('./domBase');

/**
 * 清理无用的 子节点 span ，合并 attribute & style 相同的 span
 * @param dom
 * @param excludeList
 */
domUtils.clearChild = function (dom, excludeList) {
    if (!dom) {
        return;
    }
    var isExclude = excludeList.indexOf(dom) >= 0;
    var str;
    if (!isExclude && dom.nodeType === 3 && !domUtils.isUsableTextNode(dom) &&
        !domUtils.getParentByTagName(dom, 'pre', false)) {
        // pre 内不进行清理
        domUtils.remove(dom);
        return;
    } else if (!isExclude && dom.nodeType === 3) {
        str = dom.nodeValue.replace(CONST.FILL_CHAR_REG, '');
        // ios 10.3 设置 nodeValue 会导致光标移位（str === nodeValue 时，也会移位）
        if (str !== dom.nodeValue) {
            dom.nodeValue = str;
        }
        return;
    }

    if (!isExclude && dom.nodeType === 1) {
        var ns = dom.childNodes, i, item;
        for (i = ns.length - 1; i >= 0; i--) {
            item = ns[i];
            domUtils.clearChild(item, excludeList);
        }
        domUtils.mergeChildSpan(dom, excludeList);

        if (excludeList.indexOf(dom) < 0 &&
            dom.childNodes.length === 0 && dom.nodeType === 1 && !domUtils.isSelfClosingTag(dom) &&
//                    dom.tagName.toLowerCase() == 'span' && !!dom.getAttribute(CONST.ATTR.SPAN)) {
            !!dom.getAttribute(CONST.ATTR.SPAN)) {
            domUtils.remove(dom);
        }

    }
};
/**
 * 处理 copy、cut 操作时的元素容器，清理多余内容
  */
domUtils.fragmentFilterForCopy = function (fragment) {
    // 清理 codeMirror 内多余元素
    var domList = fragment.querySelectorAll('.' + CONST.CLASS.CODE_CONTAINER + ' textarea');
    var dom, i;
    for (i = domList.length - 1; i >= 0; i--) {
        dom = domList[i];
        domUtils.remove(dom);
    }
    domList = fragment.querySelectorAll('.' + CONST.CLASS.CODE_MIRROR_MEASURE);
    for (i = domList.length - 1; i >= 0; i--) {
        dom = domList[i];
        domUtils.remove(dom);
    }
    domList = fragment.querySelectorAll('.' + CONST.CLASS.CODE_MIRROR_GUTTER);
    for (i = domList.length - 1; i >= 0; i--) {
        dom = domList[i];
        domUtils.remove(dom);
    }
    domList = fragment.querySelectorAll('.' + CONST.CLASS.CODE_TOOLS);
    for (i = domList.length - 1; i >= 0; i--) {
        dom = domList[i];
        domUtils.remove(dom);
    }
};

/**
 * 修正 fontSize
 */
domUtils.fixFontSize = function () {
    var domList = ENV.doc.querySelectorAll('[style]');
    var i, dom, style, size;
    var fontSizeReg = /font-size\s*:\s*([%.\w]*)/i, result;
    for (i = 0; i < domList.length; i++) {
        dom = domList[i];
        style = dom.getAttribute('style');
        result = fontSizeReg.exec(style);
        if (!result || result[1].indexOf('rem') > 0) {
            continue;
        }
        size = domUtils.getFontSizeRem(result[1], {
            useRootSize: true
        });
        if (!domUtils.isTag(dom, 'html')) {
          dom.style.fontSize = size;
        }
    }
};

/**
 * 修正 有序列表 ol 的 className
 */
domUtils.fixOrderList = function () {
    var orderList = ENV.doc.querySelectorAll('ol');
    var i, ol, p, level;
    for (i = 0; i < orderList.length; i++) {
        ol = orderList[i];
        p = ol;
        level = 0;
        while (p) {
            level++;
            p = domUtils.getParentByTagName(p, ['ul', 'ol'], false);
        }
        domUtils.removeClass(ol, CONST.CLASS.ORDER_LIST_LEVEL);
        domUtils.addClass(ol, CONST.CLASS.ORDER_LIST_LEVEL[level % 3]);
    }
};

/**
 * 获取 body 内的 innerText
 * @returns {*}
 */
domUtils.getBodyText = function () {
    var body = ENV.doc.body;
    if (!body) return " ";
    return body.innerText ? body.innerText : '';
};
/**
 * 获取当前页面源码
 * @param options
 * @returns {*}
 */
domUtils.getContentHtml = function (options) {
    // PC 客户端使用的 Chrome 引擎在 设置列表后，会自动出现 font-size 的样式，必须要进行修正
    domUtils.fixFontSize();

    var isSaveTemp = options && options.isSaveTemp;
    ENV.event.call(CONST.EVENT.BEFORE_GET_DOCHTML, null);

    var docType = domUtils.getDocType(ENV.doc);

    //将 input text、textarea 的内容设置到 html 内
    var i, j, obj;
    var objList = ENV.doc.getElementsByTagName('input');
    for (i = 0, j = objList.length; i < j; i++) {
        obj = objList[i];
        if (/^test$/i.test(obj.getAttribute('type')) && obj.value !== obj.getAttribute('value')) {
            obj.setAttribute('value', obj.value);
        }
    }
    objList = ENV.doc.getElementsByTagName('textarea');
    for (i = 0, j = objList.length; i < j; i++) {
        obj = objList[i];
        domUtils.setTextarea(obj);
    }
    //处理 table 容器内 table 之外的内容
    objList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TABLE_CONTAINER);
    for (i = 0, j = objList.length; i < j; i++) {
        domUtils.moveOutFromTableContainer(objList[i]);
    }

    var tmpStyle = [CONST.NAME.TMP_STYLE];
    if (!isSaveTemp) {
        tmpStyle.push(CONST.NAME.UNSAVE_STYLE);
    }
    var content = domUtils.removeStyleByNameFromHtml(ENV.doc.documentElement.outerHTML, tmpStyle);
    content = domUtils.removeByTagFromHtml(content, CONST.TAG.TMP_TAG);
    content = domUtils.peelTagFromHtml(content, CONST.TAG.TMP_PLUGIN_TAG);

    // 移除 viewport(为什么要移除 viewport ？？)
    // content = domUtils.removeViewportFromHtml(content, CONST.TAG.TMP_TAG);

    // 奇葩案例：抓取的页面注释中出现以下内容，导致 笔记被编辑后，就把全文都全部注释，导致看不到内容
    /*
       <!-- <link rel="stylesheet" href="/monthly/themes/tomorrow.css">
       <script src="/monthly/highlight/highlight.pack.js"> -->
    */
    // 所以必须先清理注释，然后再清理 script

    // 移除 注释内容
    content = content.replace(/<!--(.|\n)*?(-->)/g, '');
    // 移除 script
    content = content.replace(/<script[^<>]*\/>/ig, '')
        .replace(/<script[^<>]*>(((?!<\/script>).)|(\r?\n))*<\/script>/ig, '');

    // 需要兼容 WizTemplate 中的部分区域可编辑 状态
    //var bodyReg = /(<body( [^<>]*)*)[ ]+contenteditable[ ]*=[ ]*['"][^'"<>]*['"]/ig;
    var bodyReg = /(<[\w]*[^<>]*[ ]+)contenteditable([ ]*=[ ]*['"][^'"<>]*['"])?/ig;
    content = content.replace(bodyReg, '$1');

    // 需要处理 CodeMirror 内 的隐藏占位区域，否则会把内容当作摘要
    content = content.replace(/(<div class="CodeMirror-measure">)<pre><span>xxxxxxxxxx<\/span><\/pre>/ig, '$1');

    content = domUtils.hideTableFromHtml(content);
    content = domUtils.hideCodeFromHtml(content);

    //过滤其他插件
    if (ENV.win.WizTemplate) {
        content = ENV.win.WizTemplate.hideTemplateFormHtml(content);
    }

    // 清理特殊字符
    content = utils.replaceSpecialChar(content);

    var editingImgReg, imgFullPathReg;
    if (!isSaveTemp) {
        // Android 编辑图片时，临时保存页面，不能去掉 Editing 标识，否则无法替换 img 的 src
        editingImgReg = new RegExp('(<img[^<>]*)( ' + CONST.ATTR.IMG_EDITING + '=(\'|")1\\3)', 'ig');
        content = content.replace(editingImgReg, '$1');

        // 恢复图片 index_files 的路径
        imgFullPathReg = new RegExp('(<[^<>]*src[ ]*=[ ]*("|\'))' + ENV.options.indexFilesFullPath.escapeRegex(), 'ig');
        content = content.replace(imgFullPathReg, '$1' + ENV.options.indexFilesPath + '/');
    }
    return docType + content;
};
/**
 * 根据 node 获取其之前的第一个 块级元素（包括自己的父节点）
 * @param tmpStart
 * @returns {*}
 */
domUtils.getPrevBlock = function (tmpStart) {
    var isPre = false, tmp;
    if (domUtils.isBlock(tmpStart) || domUtils.isTag(tmpStart, 'br')) {
        return tmpStart;
    }
    while (tmpStart) {
        if (isPre && tmpStart.childNodes && tmpStart.childNodes.length > 0) {
            tmpStart = tmpStart.childNodes[tmpStart.childNodes.length - 1];
            isPre = false;
        } else {
            tmp = tmpStart.previousSibling;
            if (tmp) {
                tmpStart = tmp;
                isPre = true;
            } else {
                tmpStart = tmpStart.parentNode;
                isPre = false;
            }
        }
        if (domUtils.isBlock(tmpStart) || domUtils.isTag(tmpStart, 'br')) {
            break;
        }
    }
    return tmpStart;
};
/**
 * 根据 node 获取其之后的第一个 块级元素（包括自己的父节点）
 * @param tmpEnd
 * @returns {*}
 */
domUtils.getNextBlock = function (tmpEnd) {
    var isNext = false, tmp;

    if (domUtils.isBlock(tmpEnd) || domUtils.isTag(tmpEnd, 'br')) {
        return tmpEnd;
    }

    while (tmpEnd) {
        if (isNext && tmpEnd.childNodes && tmpEnd.childNodes.length > 0) {
            tmpEnd = tmpEnd.childNodes[0];
            isNext = false;
        } else {
            tmp = tmpEnd.nextSibling;
            if (tmp) {
                tmpEnd = tmp;
                isNext = true;
            } else {
                tmpEnd = tmpEnd.parentNode;
                isNext = false;
            }
        }
        if (domUtils.isBlock(tmpEnd) || domUtils.isTag(tmpEnd, 'br')) {
            break;
        }
    }
    return tmpEnd;
};
/**
 * 针对 html 源码隐藏 CodeMirror 光标 & 当前行 & 当前选中区域，主要用于保存操作
 * @param html
 * @returns {XML|string|*|void}
 */
domUtils.hideCodeFromHtml = function (html) {
    var regHide = /(<div [^<>]*)(CodeMirror-activeline-background|CodeMirror-cursors|CodeMirror-selected|CodeMirror-hscrollbar|CodeMirror-vscrollbar)([^<>]*>)/ig;
    html = html.replace(regHide, '$1 wiz-hide wiz_$2$3');
    var regNoClass = /(<span [^<>]*)(CodeMirror-matchingbracket)([^<>]*>)/ig;
    html = html.replace(regNoClass, '$1$3');
    return html;
};
/**
 * 针对 html 源码隐藏表格的高亮信息，主要用于保存操作
 * @param html
 * @returns {*}
 */
domUtils.hideTableFromHtml = function (html) {
    //做 RegExp 做 test 的时候 不能使用 g 全局设置， 否则会影响 索引
    var regexForTest = /(<[^<> ]*[^<>]* class[ ]*=[ ]*['"])([^'"]*)(['"])/i;
    var regex = /(<[^<> ]*[^<>]* class[ ]*=[ ]*['"])([^'"]*)(['"])/ig;
    if (!regexForTest.test(html)) {
        return html;
    }

    var result = [], m, lastIndex = 0,
        str, reg;
    while (m = regex.exec(html)) {
        str = m[2];

        //先处理 float layer
        if ((' ' + str + ' ').indexOf(' ' + CONST.CLASS.SELECTED_CELL + ' ') > -1) {
            reg = new RegExp(' ' + CONST.CLASS.SELECTED_CELL + ' ', 'ig');
            str = (' ' + str + ' ').replace(reg, '').trim();
        }

        result.push(html.substr(lastIndex, m.index - lastIndex), m[1], str, m[3]);

        lastIndex = m.index + m[0].length;
        //console.log(m);
    }
    result.push(html.substr(lastIndex));
    return result.join('');
};
/**
 * 合并 子节点中相邻且相同（style & attribute ）的 span
 * merge the same span with parent and child nodes.
 * @param dom
 * @param excludeList
 */
domUtils.mergeChildSpan = function (dom, excludeList) {
    if (!dom || dom.nodeType !== 1) {
        return;
    }
    var i, j;
    for (i = 0, j = dom.children.length; i < j; i++) {
        domUtils.mergeChildSpan(dom.children[i], excludeList);
    }
    domUtils.mergeSiblingSpan(dom, excludeList);

    var n = dom.children[0], tmp;
    if (!!n && excludeList.indexOf(n) < 0 && dom.childNodes.length == 1 &&
        dom.getAttribute(CONST.ATTR.SPAN) == CONST.ATTR.SPAN &&
        n.getAttribute(CONST.ATTR.SPAN) == CONST.ATTR.SPAN) {
        domUtils.mergeChildToParent(dom, n);
    } else {
        while (!!n) {
            if (excludeList.indexOf(n) < 0 && excludeList.indexOf(dom) < 0 && domUtils.isSameSpan(dom, n)) {
                tmp = n.previousElementSibling;
                domUtils.mergeChildToParent(dom, n);
                n = tmp ? tmp.nextElementSibling : dom.children[0];
            } else {
                n = n.nextElementSibling;
            }
        }
    }
};
/**
 * 将 子节点 合并到 父节点 （主要用于 嵌套的 span 合并）
 * @param parent
 * @param child
 */
domUtils.mergeChildToParent = function (parent, child) {
    if (!parent || !child || child.parentNode !== parent) {
        return;
    }
    while (child.childNodes.length > 0) {
        domUtils.before(child.childNodes[0], child);
    }
    domUtils.mergeAtoB(parent, child, false);
    domUtils.mergeAtoB(child, parent, true);
    parent.removeChild(child);
};
/**
 * 合并相邻且相同（style & attribute ）的 span
 * @param parentDom
 * @param excludeList
 */
domUtils.mergeSiblingSpan = function (parentDom, excludeList) {
    var n = parentDom.childNodes[0], m, tmp;
    if (!n) {
        return;
    }
    while (n) {
        m = n.nextSibling;
        if (m && excludeList.indexOf(m) < 0 && excludeList.indexOf(n) < 0 && domUtils.isSameSpan(n, m)) {
            while (m.childNodes.length) {
                tmp = m.childNodes[0];
                if (tmp && (tmp.innerHTML || (tmp.nodeValue && tmp.nodeValue != CONST.FILL_CHAR))) {
                    n.appendChild(tmp);
                } else {
                    m.removeChild(tmp);
                }
            }
            domUtils.remove(m);
        } else {
            n = m;
        }
    }
};
domUtils.modifyChildNodesStyle = function (dom, style, attr) {
    if (!dom) {
        return;
    }
    var ns = dom.childNodes, done = false, i, item;
    for (i = 0; i < ns.length; i++) {
        item = ns[i];
        if (!done && domUtils.isUsableTextNode(item)) {
            done = true;
            domUtils.modifyStyle(dom, style, attr);
        } else if (item.nodeType === 1) {
            domUtils.modifyChildNodesStyle(item, style, attr);
        }
    }
};
domUtils.modifyNodeStyle = function (item, style, attr, isLast) {
    if (item.nodeType === 1) {
        if (domUtils.isSelfClosingTag(item)) {
            domUtils.modifyStyle(item, style, attr);
        } else {
            domUtils.modifyChildNodesStyle(item, style, attr);
        }

    } else if (domUtils.isUsableTextNode(item)) {
        item = domUtils.splitRangeText(item, null, null, isLast);
        domUtils.modifyStyle(item, style, attr);
    }
    return item;
};
/**
 * 修改 集合中所有Dom 的样式（style） & 属性（attribute）
 * @param domList
 * @param style
 * @param attr
 */
domUtils.modifyNodesStyle = function (domList, style, attr) {
    if (domList.length === 0) {
        return;
    }
    var i, j, item;
    for (i = 0, j = domList.length; i < j; i++) {
        item = domList[i];
        domList[i] = domUtils.modifyNodeStyle(item, style, attr, i === j - 1);
    }
};
/**
 * 修改 Dom 的样式（style） & 属性（attribute）
 * @param dom
 * @param style
 * @param attr
 */
domUtils.modifyStyle = function (dom, style, attr) {
    var isSelfClosingTag = domUtils.isSelfClosingTag(dom);
    var isTodoCheckbox = domUtils.hasClass(dom, CONST.CLASS.TODO_CHECKBOX);
    if (isTodoCheckbox) {
        // TodoCheckbox 禁止通过此方法修改样式和属性
        return
    }
    // 自闭合标签 不允许设置 新增的修订标识
    if (attr && attr[CONST.ATTR.SPAN_INSERT] && isSelfClosingTag) {
        return;
    }

    var d = dom;

    if (attr && (attr[CONST.ATTR.SPAN_INSERT] || attr[CONST.ATTR.SPAN_DELETE])) {
        // 如果 dom 是 修订的内容， 且设定修订内容 则必须要针对 修订DOM 处理
        d = domUtils.getWizAmendParent(dom);
        if (!d) {
            d = dom;
        } else {
            dom = null;
        }
    }

    if (!!dom && !isSelfClosingTag &&
        (!domUtils.isTag(dom, 'span') || dom.getAttribute(CONST.ATTR.SPAN) !== CONST.ATTR.SPAN)) {
        d = domUtils.createSpan();
        dom.insertBefore(d, null);
        while (dom.childNodes.length > 1) {
            d.insertBefore(dom.childNodes[0], null);
        }
    }

    // 遍历 style ，检查是否有需要清空的样式
    var k, v, hasClearList = [];
    for (k in style) {
        if (style.hasOwnProperty(k)) {
            v = style[k];
            if (!v) {
                hasClearList.push(k);
            }
        }
    }
    // 如果有 需要清空的 style，则需要拆分父节点
    var tmpP = d, tmpA, p = d;
    if (hasClearList.length > 0) {
        // 找到 行级元素顶级节点
        while (tmpP) {
            // if (tmpP.getAttribute(CONST.ATTR.SPAN) !== CONST.ATTR.SPAN) {
            // block 或 TodoMain 的 span 都禁止拆分
            if (domUtils.isBlock(tmpP) || domUtils.hasClass(tmpP, CONST.CLASS.TODO_MAIN)) {
                break;
            }
            if (domUtils.isTag(tmpP, 'a')) {
                tmpA = tmpP;
            }
            p = tmpP;
            tmpP = p.parentNode;
        }
        // 如果父节点有 A 标签，需要特殊处理，避免 清理样式时 A 标签被拆分成多个
        if (!tmpA) {
            domUtils.splitDomSingle(p, d);
        } else if (tmpA === p) {
            filterForA(tmpA);
            domUtils.splitDomSingle(tmpA.firstChild, d);
        } else {
            domUtils.splitDomSingle(p, tmpA);
            filterForA(tmpA);
            domUtils.splitDomSingle(tmpA.firstChild, d);
        }
    }
    domUtils.css(d, style);
    domUtils.attr(d, attr);

    // 把 A 标签的子节点封装到一个 span 内，用于拆分
    function filterForA (aObj) {
        if (!aObj) {
            return;
        }
        var span = domUtils.createSpan();
        span.setAttribute('style', aObj.getAttribute('style') || '');
        aObj.removeAttribute('style');
        while (aObj.firstChild) {
            span.insertBefore(aObj.firstChild, null);
        }
        aObj.insertBefore(span, null);
    }
};
/**
 * 将 table 容器内 非 table 内容移出
 * @param container
 */
domUtils.moveOutFromTableContainer = function (container) {
    if (!container) {
        return;
    }

    move(container, container);
    move(container, container.querySelector('.' + CONST.CLASS.TABLE_BODY));

    function move (mainDom, _container) {
        if (!mainDom || !_container) {
            return;
        }

        var isInContainer = mainDom === _container;
        var childList, dom, target, i,
            before = false, hasTableChild = false;
        childList = _container.childNodes;
        target = mainDom;
        for (i = childList.length - 1; i >= 0; i--) {
            dom = childList[i];
            // 遍历 chlidren 时，在 table 之前 before 为 true；在 table 之后 before 为 false
            if (dom.nodeType === 1 &&
                ((isInContainer && domUtils.hasClass(dom, CONST.CLASS.TABLE_BODY)) ||
                    (!isInContainer && domUtils.isTag(dom, ['table', CONST.TAG.TMP_TAG])))) {
                if (!hasTableChild) {
                    // hasTableChild 必须要进行判断，
                    // 否则由于用户反复编辑有可能会出现 一个 container 下有多个 tableBody 的情况，从而导致渲染后 table 消失
                    if (domUtils.hasClass(dom, CONST.CLASS.TABLE_BODY) || domUtils.isTag(dom, 'table')) {
                        before = true;
                        hasTableChild = true;
                    }
                    continue;
                }
            }
            domUtils.before(dom, target, !before);
            // 往前放的时候， 必须每次更换 target，否则 DOM 顺序会错误
            if (before) {
                target = dom;
            }
        }

        if (_container.childNodes.length === 0) {
            // 如果 table 容器内清空，则删除 容器
            domUtils.remove(mainDom);
        }
    }
};
/**
 * 在删除 当前用户已删除 指定的Dom 后， 判断其 parentNode 是否为空，如果为空，继续删除
 * @param pDom
 */
domUtils.removeEmptyParent = function (pDom) {
    if (!pDom) {
        return;
    }
    var p;
    if (domUtils.isEmptyDom(pDom)) {
        if (pDom === ENV.doc.body || domUtils.isTag(pDom, ['td', 'th'])) {
            //如果 pDom 为 body | td | th 且为空， 则添加 br 标签
            pDom.innerHTML = '<br/>';
        } else {
            p = pDom.parentNode;
            if (p) {
                p.removeChild(pDom);
                domUtils.removeEmptyParent(p);
            }
        }
    }
};

/**
 * 将 mainDom 以子节点 subDom 之前为分割点 分割为两个 mainDom（用于 修订处理）
 * 返回 subDom 所在的 mainDom
 * @param mainDom
 * @param subDom
 */
domUtils.splitDomBeforeSub = function (mainDom, subDom) {
    if (!mainDom || !subDom) {
        return mainDom;
    }
    var parents = [subDom], p = subDom.parentNode;
    var isFirstChild = p.firstChild === subDom;
    while (p) {
        parents.push(p);
        if (isFirstChild && p.parentNode.firstChild !== p) {
            isFirstChild = false;
        }
        if (p === mainDom) {
            break;
        }
        p = p.parentNode;
    }
    if (isFirstChild) {
        // 如果 subDom 每一个父节点（包括 subDom） 都是 firstChild， 则不进行拆分
        return mainDom;
    }

    // 如果 subDom 不是 mainDom 的子孙节点直接返回
    if (parents[parents.length - 1] !== mainDom) {
        return mainDom;
    }
    while (parents.length > 1) {
        parents[1] = split(parents[1], parents[0]);
        parents.splice(0, 1);
    }

    return parents[0];

    function split (parent, child) {
        var p = parent.parentNode,
            pCopy = parent.cloneNode(false),
            next;

        while (child) {
            next = child.nextSibling;
            pCopy.appendChild(child);
            child = next;
        }
        p.insertBefore(pCopy, parent.nextSibling);
        return pCopy;
    }
};

/**
 * 将 mainDom 以子节点 subDom 之后为分割点 分割为两个 mainDom（用于 修订处理）
 * 返回 subDom 所在的 mainDom
 * @param mainDom
 * @param subDom
 */
domUtils.splitDomAfterSub = function (mainDom, subDom) {
    var tmpP, next;

    // 拆分 subDom 后面的内容
    if (!subDom.nextSibling) {
        tmpP = subDom.parentNode;
        while (tmpP && tmpP !== mainDom) {
            // if (tmpP.getAttribute(CONST.ATTR.SPAN) !== CONST.ATTR.SPAN) {
            // if (domUtils.isBlock(tmpP)) {
            //     break;
            // }
            next = tmpP.nextSibling;
            if (!!next) {
                break;
            }
            tmpP = tmpP.parentNode;
        }
    } else {
        next = subDom.nextSibling;
    }
    if (next) {
        domUtils.splitDomBeforeSub(mainDom, next);
    }
    return mainDom;
};

/**
 * 将 mainDom 以子节点 subDom 前后为分割点，把 subDom 独立于 前后 元素独立拆分成 3 段，并且保持 Dom 结构
 * @param mainDom
 * @param subDom
 */
domUtils.splitDomSingle = function (mainDom, subDom) {
    if (mainDom && subDom && mainDom.nodeType === 1 &&
        subDom === mainDom.firstChild && mainDom.childNodes.length === 1) {
        return mainDom;
    }

    // 拆分 subDom 前面的内容
    var mainP = domUtils.splitDomBeforeSub(mainDom, subDom);
    mainP = domUtils.splitDomAfterSub(mainP, subDom);
    return mainP;
};

module.exports = domUtils;

},{"./../common/const":18,"./../common/env":20,"./../common/utils":24,"./domBase":28}],30:[function(require,module,exports){
/**
 * 下拉框插件
 */

var ENV = require('../common/env'),
    CONST = require('../common/const'),
    domUtils = require('./domBase');

var _class = {
    active: 'active',
    selected: 'selected'
};

var _event = {
    bind: function () {
        // 选项不采用 click 方式，主要为了避免 与 CodeMirror 抢焦点
        // 组件中不进行 阻止默认事件 的操作（具体实现中根据情况进行控制，例如：codeCore 中的 onMouseDown）
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
    },
    handler: {
        onMouseDown: function (e) {
            var target = e.target;
            var header = selectUtils.getHeaderFromDom(target);
            if (header) {
                selectUtils.showOptions(header);
                return;
            }
            var option = selectUtils.getOptionFromDom(target);
            if (option) {
                selectUtils.selectOption(option);
            }
            var container = selectUtils.getContainerFromDom(target);
            if (!container) {
                selectUtils.hideOptions();
            }
        }
    }
};

var selectUtils = {
    getContainerFromDom: function (dom) {
        return domUtils.getParentByClass(dom, CONST.CLASS.SELECT_PLUGIN_CONTAINER, true);
    },
    getHeaderFromDom: function (dom) {
        return domUtils.getParentByClass(dom, CONST.CLASS.SELECT_PLUGIN_HEADER, true);
    },
    getHeaderText: function (container) {
        return container.querySelector('.' + CONST.CLASS.SELECT_PLUGIN_HEADER_TEXT);
    },
    getOptionFromDom: function (dom) {
        return domUtils.getParentByClass(dom, CONST.CLASS.SELECT_PLUGIN_OPTIONS_ITEM, true);
    },
    hideOptions: function (container) {
        var containerList, i;
        if (!container) {
            containerList = ENV.doc.querySelectorAll('.' + CONST.CLASS.SELECT_PLUGIN_CONTAINER + '.' + _class.active);
        } else {
            containerList = [container];
        }
        for (i = containerList.length - 1; i >= 0; i--) {
            domUtils.removeClass(containerList[i], _class.active);
        }
    },
    selectOption: function(option) {
        var container = selectUtils.getContainerFromDom(option);
        if (!container) {
            return;
        }
        var selectedItem = container.querySelectorAll('.' + CONST.CLASS.SELECT_PLUGIN_OPTIONS_ITEM + '.' + _class.selected);
        if (selectedItem === option) {
            return;
        }
        container.value = option.getAttribute('value');

        domUtils.removeClass(selectedItem, _class.selected);
        domUtils.addClass(option, _class.selected);

        var headerText = selectUtils.getHeaderText(container);
        headerText.textContent = option.textContent;
        selectUtils.hideOptions(container);

        ENV.event.call(CONST.EVENT.ON_SELECT_PLUGIN_CHANGE, container);
    },
    showOptions: function (header) {
        var container = selectUtils.getContainerFromDom(header);
        if (domUtils.hasClass(container, _class.active)) {
            selectUtils.hideOptions(container);
            return;
        }
        selectUtils.hideOptions();
        domUtils.addClass(container, _class.active);
    }
};

var selectPlugin = {
    on: function () {
        _event.bind();
    },
    off: function () {
        _event.unbind();
    },
    create: function (optionsData, selectClass, defaultValue) {
        var container = ENV.doc.createElement('div');
        var header = ENV.doc.createElement('div');
        var headerText = ENV.doc.createElement('span');
        var headerIcon = ENV.doc.createElement('i');
        var options = ENV.doc.createElement('ul');
        var mockOption;
        var i, j, text, value;

        header.appendChild(headerText);
        header.appendChild(headerIcon);
        container.appendChild(header);
        container.appendChild(options);

        for (i = 0, j = optionsData.length; i < j; i++) {
            text = optionsData[i].text;
            value = optionsData[i].value;
            mockOption = ENV.doc.createElement('li');
            mockOption.textContent = text;
            domUtils.attr(mockOption, {'value': value, 'data-index': i});
            if (value === defaultValue) {
                headerText.textContent = text;
                container.value = value;
                domUtils.addClass(mockOption, _class.selected);
            }
            domUtils.addClass(mockOption, CONST.CLASS.SELECT_PLUGIN_OPTIONS_ITEM);
            options.appendChild(mockOption);
        }
        domUtils.addClass(headerIcon, 'icon-down_arrow editor-icon');
        domUtils.addClass(headerText, CONST.CLASS.SELECT_PLUGIN_HEADER_TEXT);
        domUtils.addClass(header, CONST.CLASS.SELECT_PLUGIN_HEADER);
        domUtils.addClass(options, CONST.CLASS.SELECT_PLUGIN_OPTIONS);
        domUtils.addClass(container, CONST.CLASS.SELECT_PLUGIN_CONTAINER + ' ' + selectClass);
        domUtils.attr(container, {
            'onselectstart': 'return false;'
        });
        return container;
    },
    destory: function () {

    }
};

module.exports = selectPlugin;

},{"../common/const":18,"../common/env":20,"./domBase":28}],31:[function(require,module,exports){
/**
 * xss 工具包
 */
var filterXSS = require('../common/xss');

var xssUtils = {
    xssFilter: (function () {
        if (typeof filterXSS == 'undefined') {
            return null;
        }
        var fileName = decodeURIComponent(location.pathname),
            fileNameStart = fileName.lastIndexOf('/') + 1,
            fileNameEnd = fileName.lastIndexOf('.');
        fileName = fileNameEnd > 0 ? fileName.substring(fileNameStart, fileNameEnd) + '_files' : '';

        // var imgSrc = 'file://|data:image/';
        // var imgSrcReg = new RegExp('^(' + imgSrc + fileName + ')', 'i');
        // var imgSrcReg = /^javascript:/i;

        var hrefReg = /^((file|wiz(note)?):\/\/)|(index_files\/)|(data:image\/(?!svg))/;
        var hrefFileSrc = '#';
        var hrefFileReg = new RegExp('^(' + hrefFileSrc.escapeRegex() +
          (fileName ? ('|' + fileName.escapeRegex()) : '') + ')', 'i');

      var tagAttrReg = /^(id|class|name|style|data|width|height)/i;

        var xss = new filterXSS.FilterXSS({
            onIgnoreTag: function (tag, html, options) {
                //针对白名单之外的 tag 处理
                if (/script/ig.test(tag)) {
                    return filterXSS.escapeAttrValue(html);
                }
                if (options.isClosing) {
                    return '</' + tag + '>';
                }

                var x = filterXSS.parseAttr(html, function (name, value) {
                    value = xss.options.safeAttrValue(tag, name, value, xss);
                    if (/^on/i.test(name)) {
                        return '';
                    } else if (value) {
                        return name + '="' + value + '"';
                    } else {
                        return name;
                    }
                });

                if (/^<!/i.test(html)) {
                    //<!doctype html>
                    x = '<!' + x;
                } else {
                    x = '<' + x;
                }

                if (html[html.length - 2] === '/') {
                    x += '/';
                }
                x += '>';
                return x;
            },
            onIgnoreTagAttr: function (tag, name, value, isWhiteAttr) {
                if (!/^object$/i.test(tag) && !!value && tagAttrReg.test(name)) {
                    return name + '="' + value + '"';
                }
                return '';
            },
            safeAttrValue: function (tag, name, value) {
                if (/^meta$/i.test(tag)) {
                    if (/^http-equiv$/i.test(name) && /refresh/i.test(value)) {
                        return '';
                    }
                }
                // 自定义过滤属性值函数，如果为a标签的href属性，则先判断是否以wiz://开头
                if (name === 'href' || name === 'src') {
                    if (hrefReg.test(value) || hrefFileReg.test(value)) {
                        return filterXSS.escapeAttrValue(value);
                    }
                }
                // 其他情况，使用默认的safeAttrValue处理函数
                return filterXSS.safeAttrValue(tag, name, value);
            }
        });

        // 增加白名单
        xss.options.whiteList.iframe = ['src', 'scrolling'];
        xss.options.whiteList.button = ['title', 'type', 'value'];
        xss.options.whiteList.object = [];

        return function (html) {
            return xss.process(html);
        };

    })()
};

module.exports = xssUtils;

},{"../common/xss":27}],32:[function(require,module,exports){
/**
 * 编辑器 基础工具包
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    dependLoader = require('../common/dependLoader'),
    historyUtils = require('../common/historyUtils'),
    wizStyle = require('../common/wizStyle'),
    amendUtils = require('../amend/amendUtils/amendExtend'),
    amendUser = require('../amend/amendUser'),
    amend = require('../amend/amend'),
    blockCore = require('../blockUtils/blockCore'),
    codeCore = require('../codeUtils/codeCore'),
    domUtils = require('../domUtils/domExtend'),
    selectPlugin = require('../domUtils/selectPlugin'),
    commandExtend = require('../editor/commandExtend'),
    formatPainterCore = require('../formatPainterUtils/formatPainterCore'),
    imgCore = require('../imgUtils/imgCore'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    tableCore = require('../tableUtils/tableCore'),
    tableUtils = require('../tableUtils/tableUtils'),
    tableZone = require('../tableUtils/tableZone'),
    todoCore = require('../todoUtils/todoCore'),
    editorEvent = require('./editorEvent'),
    tabKey = require('./tabKey');

var originalHtml = '';
var editor = {
    init: function () {

    },
    on: function (callback) {
      // if (ENV.client.type.isAndroid || ENV.client.type.isIOS) {
      //   // 手机客户端 需要主动加上 viewport
      //   if (!ENV.doc.querySelector('meta[name=viewport]')) {
      //     var head = ENV.doc.querySelector('head');
      //     var meta = ENV.doc.createElement('meta');
      //     meta.setAttribute('name', 'viewport');
      //     meta.setAttribute('content', 'width=device-width, initial-scale=1.0, maximum-scale=1.0, minimum-scale=1.0, user-scalable=no');
      //     head.appendChild(meta);
      //   }
      // }

        dependLoader.loadCss(ENV.doc, [ENV.dependency.files.css.fonts]);
        domUtils.setContenteditable(null, true);
        ENV.readonly = false;
        wizStyle.insertTmpEditorStyle();

        codeCore.loadDependency(_callback);

        function _callback() {
            domUtils.fixOrderList();
            editorEvent.bind();
            commandExtend.on();
            selectPlugin.on();
            imgCore.on();
            tableCore.on();
            tabKey.on();
            todoCore.on();
            blockCore.on();
            codeCore.on();
            formatPainterCore.init();

            amend.startReverse();
            historyUtils.start(ENV.options.maxRedo, ENV.options.callback.redo);
            imgCore.setImgFullPath();

            if (ENV.client.type.isPhone || ENV.client.type.isPad) {
                //手机端 渲染可能会慢，所以必须要延时处理
                setTimeout(function () {
                    editor.setOriginalHtml();
                }, 500);
            } else {
                editor.setOriginalHtml();
            }

            if (typeof callback === 'function') {
                callback();
            }
        }
    },
    off: function () {
        historyUtils.stop();
        amend.stopReverse();
        amend.stop();
        formatPainterCore.off();
        codeCore.off();
        blockCore.off();
        todoCore.off();
        tabKey.off();
        tableCore.off();
        imgCore.off();
        selectPlugin.off();
        commandExtend.off();
        editorEvent.unbind();

        ENV.readonly = true;
        domUtils.setContenteditable(null, false);
        domUtils.removeByName(CONST.NAME.TMP_STYLE);
        domUtils.removeByTag(CONST.TAG.TMP_TAG);
    },
    find: function (str, matchcase, searchBackward, loop) {
        if (!str) {
            return false;
        }
        var result = ENV.win.find(str, matchcase, searchBackward);
        if (!result && !!loop) {
            rangeUtils.setRange(ENV.doc.body, searchBackward ? ENV.doc.body.childNodes.length : 0);
            result = ENV.win.find(str, matchcase, searchBackward);
        }
        return result;
    },
    getOriginalHtml: function () {
        return originalHtml;
    },
    insertDom: function (domList, notInsertEmptyDiv) {
        if (!domList) {
            return;
        }
        var tmpDom = readyForInsert(), i, j,
            tmpParent, dom,
            lastDom, imgLast, tmpImgLast;

        if (notInsertEmptyDiv && domUtils.isEmptyDom(tmpDom.parent)
            && !domUtils.isTag(tmpDom.parent, ['td', 'th'])
            && !tmpDom.target) {
            tmpParent = tmpDom.parent;
            tmpDom.parent = tmpParent.parentNode;
            tmpDom.target = tmpParent.nextSibling;
            tmpDom.parent.removeChild(tmpParent);
        }
        if (!utils.isArray(domList)) {
            domList = [domList];
        }
        for (i = 0, j = domList.length; i < j; i++) {
            dom = domList[i];
            if (domUtils.isBlock(dom)) {
                getBlockEnd(dom);
            }
            tmpDom.parent.insertBefore(domList[i], tmpDom.target);
            tmpImgLast = getImg(domList[i]);
            if (tmpImgLast) {
                imgLast = tmpImgLast;
            }
            lastDom = domList[i];
        }

        // 尽量避免 插入Dom 时，造成的 div 反复嵌套，在 结尾处，必须找到最后有 nextSibling 的父节点
        function getBlockEnd(result) {
            if (result.parent && !result.target) {
                while(result.parent !== ENV.doc.body && !result.target) {
                    result.target = result.parent.nextSibling;
                    result.parent = result.parent.parentNode;
                }
            }
        }

        afterInsert(lastDom);
        // 浏览器插入 img 后，渲染为异步过程，针对最后一张图片 load 事件修正，避免 afterInsert 无效
        if (imgLast) {
            imgLast.addEventListener('load', function() {
                afterInsert(lastDom);
            });
        }

        function getImg(tmpDom) {
            if (tmpDom.nodeType !== 1) {
                return null;
            }
            var tmpImgs;
            if (domUtils.isTag(tmpDom, 'img')) {
                return tmpDom;
            } else {
                tmpImgs = tmpDom.querySelectorAll('img');
                if (tmpImgs.length > 0) {
                    return tmpImgs[tmpImgs.length - 1];
                }
            }
            return null;
        }
    },
    insertHtml: function (html, notInsertEmptyDiv) {
        if (!html) {
            return;
        }
        var template = ENV.doc.createElement('div'),
            i, j, doms = [];
        template.innerHTML = html;
        for (i = 0, j = template.childNodes.length; i < j; i++) {
            doms.push(template.childNodes[i]);
        }
        editor.insertDom(doms, notInsertEmptyDiv);
        template = null;
        ENV.event.call(CONST.EVENT.AFTER_INSERT_DOM);
    },
    modifySelectionDom: function (style, attr) {
        if (!tableCore.modifySelectionDom(style, attr)) {
            // 处理普通文本样式
            rangeUtils.modifySelectionDom(style, attr);
        }
    },
    replace: function (from, to, matchcase) {
        if (!from) {
            return false;
        }
        var selectedTxt = getSelectedTxt(), span, txt;
        if (selectedTxt == from) {
            //替换
            if (!to) {
                to = '';
            } else {
                txt = ENV.doc.createTextNode(to);
                span = ENV.doc.createElement('span');
                span.appendChild(txt);
                to = span.innerHTML;
                span = null;
                txt = null;
            }
            commandExtend.execCommand('insertHTML', false, to);
        }

        // 查找下一个
        return editor.find(from, matchcase);
    },
    replaceAll: function (from, to, matchcase) {
        if (!from) {
            return false;
        }
        rangeUtils.setRange(ENV.doc.body, 0);
        while(editor.replace(from, to, matchcase)) {}
    },
    setOriginalHtml: function () {
        originalHtml = domUtils.getContentHtml();
    }
};

function getSelectedTxt() {
    return ENV.doc.getSelection().toString();
}

/**
 * 插入内容前准备工作
 */
function readyForInsert() {
    var sel = ENV.doc.getSelection(),
        range = rangeUtils.getRange(),
        startDom, startOffset,
        result = {
            parent: null,
            target: null
        },
        span, cannotInsertDom;

    if (!range) {
        //如果页面没有焦点， 则尝试恢复光标位置， 失败后自动让 body 获取焦点
        if (!rangeUtils.restoreCaret()) {
            domUtils.focus();
        }
    }

    if (amend.isAmendEditing()) {
        //修订编辑模式下，先做修订的删除操作
        if (!range.collapsed) {
            amendUtils.removeSelection(amendUser.getCurUser());
            amendUtils.removeUserDel(null, amendUser.getCurUser());
            sel.collapseToEnd();
        }
    }

    // Iphone 端 添加 附件后，光标定位在附件的 <a> 标签后，添加图片会把图片添加到 <a> 内
    // 下面代码主要用于修正这种 bug
    range = rangeUtils.getRange();
    if (range.collapsed) {
        startDom = range.startContainer;
        startOffset = range.startOffset;
        // 判断光标是否在 附件的 <a> 标签内
        cannotInsertDom = domUtils.getParentByFilter(startDom, function (node) {
            if (node.nodeType !== 1) {
                return false;
            }
            var href = node.getAttribute('href');
            if (href && /^wiz:\/*open_attachment\?/i.test(href)) {
                return true;
            }
            return false;
        }, true);
        if (cannotInsertDom) {
            span = domUtils.createSpan();
            span.innerHTML = CONST.FILL_CHAR;
            if (startOffset === 0) {
                // 如果 光标在 <a> 前面 则将 span 插入到 <a> 前面
                domUtils.before(span, cannotInsertDom);
                rangeUtils.setRange(span, 0);
            } else {
                // 如果 光标在 <a> 中间 或 后面，则将 span 插入到 <a> 后面
                domUtils.after(span, cannotInsertDom);
                rangeUtils.setRange(span, 1);
            }
        }
    }

    //TODO 目前暂不考虑 插入 dom 和 html 时进行修订处理，仅保证不影响修订dom
    var fixed = amendUtils.fixedAmendRange();
    var newDom = amend.splitAmendDomByRange(fixed);

    range = rangeUtils.getRange();
    startDom = range.startContainer;
    startOffset = range.startOffset;

    if (newDom) {
        //直接找到新节点位置
        result.target = newDom;
        result.parent = newDom.parentNode;
    } else if (startDom.nodeType == 3 &&
        startOffset > 0 && startOffset < startDom.nodeValue.length) {
        //处于 textNode 的中间
        result.target = domUtils.splitRangeText(startDom, startOffset, null, false);
        result.parent = result.target.parentNode;
    } else if (startDom.nodeType == 1 &&
        startOffset > 0 && startOffset < startDom.childNodes.length) {
        //处于 element 节点中间
        result.target = startDom.childNodes[startOffset];
        result.parent = startDom;
    } else if (startDom == ENV.doc.body || startDom == ENV.doc.body.parentNode) {
        //处于 body 的根位置
        result.target = startOffset === 0 ? ENV.doc.body.childNodes[0] : null;
        result.parent = ENV.doc.body;
    } else if (startOffset === 0) {
        //处于 某 dom 的开始
        result.target = startDom;
        result.parent = startDom.parentNode;
    } else if (startDom.nodeType === 3) {
        //处于 textNode 的结尾
        result.target = startDom.nextSibling;
        result.parent = startDom.parentNode;
    } else {
        //处于 element 的结尾
        result.target = null;
        result.parent = startDom;
    }

    //如果下一个是 element 节点，并且为空， 则直接将内容写入到 该 element 内
    //主要针对 <div><br/></div>
    if (result.target && result.target.nodeType === 1 && !domUtils.isSelfClosingTag(result.target) &&
        domUtils.isEmptyDom(result.target)) {
        result.parent = result.target;
        if (domUtils.isTag(result.parent.childNodes[0], 'br')) {
            //添加 DOM 在 br 前需要将 br 删除
            result.parent.removeChild(result.parent.childNodes[0]);
            result.target = result.parent.childNodes.length > 0 ? result.parent.childNodes[0] : null;
        } else {
            result.target = result.parent.childNodes[0];
        }
    }

    if (domUtils.isWizDom(result.parent)) {
        // 不能把 Wiz 专用 dom 当作 parent ，否则会导致 DOM 被插入到 Wiz 专用 Dom 内
        result.target = result.parent;
        result.parent = result.parent.parentNode;
    }
    return result;
}

/**
 * 插入内容后 设置光标到插入内容结尾，并滚动到视图
 * @param lastNode
 */
var afterInsertTimer;
function afterInsert(lastNode) {
    //Preserve the selection
    if (!lastNode) {
        return;
    }
    if (afterInsertTimer) {
        clearTimeout(afterInsertTimer);
    }
    var rangTimer = 30,
        scrollTimer = 30;

    if (ENV.client.type.isPhone) {
       rangTimer = 100;
       scrollTimer = 300;
    }

    afterInsertTimer = setTimeout(function () {
        var start, target = lastNode;
        if (domUtils.isSelfClosingTag(lastNode)) {
            target = target.parentNode;
            start = domUtils.getIndex(lastNode) + 1;
        } else {
            start = domUtils.getEndOffset(lastNode);
        }

        if (domUtils.isEmptyDom(target)) {
            //避免 br 堆积
            start = 0;
        }

        rangeUtils.setRange(target, start, null, null);

        if (lastNode.nodeType === 1) {
            afterInsertTimer = setTimeout(function () {
              rangeUtils.fixScroll();
              // lastNode.scrollIntoViewIfNeeded();
            }, scrollTimer);
        }
    }, rangTimer);
}

module.exports = editor;
},{"../amend/amend":7,"../amend/amendUser":9,"../amend/amendUtils/amendExtend":11,"../blockUtils/blockCore":12,"../codeUtils/codeCore":14,"../common/const":18,"../common/dependLoader":19,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../common/wizStyle":25,"../domUtils/domExtend":29,"../domUtils/selectPlugin":30,"../editor/commandExtend":33,"../formatPainterUtils/formatPainterCore":37,"../imgUtils/imgCore":40,"../rangeUtils/rangeExtend":49,"../tableUtils/tableCore":52,"../tableUtils/tableUtils":54,"../tableUtils/tableZone":55,"../todoUtils/todoCore":56,"./editorEvent":34,"./tabKey":36}],33:[function(require,module,exports){
/**
 * 代码区域操作核心包 core
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    historyUtils = require('../common/historyUtils'),
    utils = require('../common/utils'),
    domUtils = require('../domUtils/domExtend'),
    todoUtils = require('../todoUtils/todoUtils'),
    rangeUtils = require('../rangeUtils/rangeExtend');


// 根据当前 选择范围获取 h1 - h6 集合
function getHBlockList () {
    var hList = [];
    var range = rangeUtils.getRange();
    if (!range) {
        return hList;
    }
    var start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset).container;
    var end = rangeUtils.getRangeDetail(range.endContainer, range.endOffset).container;

    var startBlock = domUtils.getBlockParent(start, true);
    if (startBlock && startBlock !== ENV.doc.body) {
        start = startBlock;
    }
    var parent = (start === end) ? start : domUtils.getParentRoot([start, end]);
    if (parent !== ENV.doc.body) {
        parent = domUtils.getBlockParent(parent, true);
    }
    if (domUtils.isTag(parent, ['h1', 'h2', 'h3', 'h4', 'h5', 'h6'])) {
        hList.push(parent);
    }

    var startIndex = domUtils.getIndexList(start).join('.');
    var endIndex = domUtils.getIndexList(end).join('.');
    var i;
    for (i = 1; i < 7; i++) {
        hList = hList.concat(domUtils.getListFromTagAndIndex('h' + i, parent, startIndex, endIndex));
    }
    return hList;
}
function getBlockList () {
    var bList = [];
    var range = rangeUtils.getRange();
    if (!range) {
        return bList;
    }
    var start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset).container;
    var end = rangeUtils.getRangeDetail(range.endContainer, range.endOffset).container;
    var startBlock = domUtils.getBlockParent(start, true);
    var endBlock = domUtils.getBlockParent(end, true);
    var endIndex = domUtils.getIndexList(end).join('.');

    var target = startBlock, next;
    var childState;
    var isBlock, isEmptyText,
        isTodo, isCode, isTable, isWizDom,
        lineStart, lineEnd, isLineEnd;
    var targetIndex;
    while (target) {
        isLineEnd = false;
        isEmptyText = false;
        isTodo = false;
        isCode = false;
        isTable = false;
        isWizDom = false;
        isBlock = domUtils.isBlock(target);

        // target 是块级元素
        targetIndex = domUtils.getIndexList(target).join('.');
        if (utils.compareVersion(targetIndex, endIndex) > 0 &&
            (isBlock || !lineStart)) {
            // 到达结尾处需要跳出循环
            break;
        }

        if (!isBlock) {
            // target 不是块级元素
            isEmptyText = target.nodeType === 3 ? utils.isEmpty(target.nodeValue) : false;
            if (!lineStart && !isEmptyText) {
                lineStart = target;
            }
            next = target.nextSibling;
            if (next) {
                target = next;
                continue;
            } else {
                isLineEnd = true;
            }
        }

        if (lineStart && (isLineEnd || isBlock)) {
            // target 之前有行级元素，但 target 是块级元素
            // 或在同级元素中到达最后一个 行级元素
            lineEnd = target;
            target = ENV.doc.createElement('div');
            if (isLineEnd) {
                domUtils.after(target, lineEnd);
            } else {
                domUtils.before(target, lineEnd);
            }
            while (lineStart && lineStart !== target) {
                target.appendChild(lineStart);
                lineStart = lineStart.nextSibling;
            }
            bList.push(target);
        } else if (isBlock) {
            isCode = !!domUtils.getParentByClass(target, CONST.CLASS.CODE_CONTAINER, true);
            if (!isCode) {
                isWizDom = domUtils.isWizDom(target);
                if (!isWizDom) {
                    isTable = !!domUtils.getParentByClass(target, CONST.CLASS.TABLE_CONTAINER, true);
                }
            }

            // isWizDom or isCode or isTable 直接跳过
            if (!isWizDom && !isCode && !isTable) {
                isTodo = !!todoUtils.isLayer(target);
                if (isTodo) {
                    bList.push(target);
                } else {
                    childState = checkBlockChild(target);
                    if (!childState.hasBlock && childState.hasLine) {
                        if (domUtils.isTag(target, 'li')) {
                            // li 需要特殊处理
                            next = ENV.doc.createElement('div');
                            domUtils.after(next, target);
                            while (target.firstChild) {
                                next.appendChild(target.firstChild);
                            }
                            target.appendChild(next);
                            target = next;
                        }

                        bList.push(target);
                    } else {
                        target = target.firstChild;
                        continue;
                    }
                }
            }
        }
        // target 被加入 集合，寻找下一个 target
        lineStart = null;
        lineEnd = null;

        next = target.nextSibling;
        while (!next && target !== ENV.doc.body && target.parentNode) {
            target = target.parentNode;
            next = target.nextSibling;
        }
        target = next;
    }
    return bList;
}

function checkBlockChild (dom) {
    var childNodes = dom.childNodes || [];
    var i, j, child;
    var hasBlockChild = false;
    var hasLineChild = false;
    for (i = 0, j = childNodes.length; i < j; i++) {
        child = childNodes[i];
        if (domUtils.isBlock(child)) {
            hasBlockChild = true;
        } else if (child.nodeType === 1 ||
            (child.nodeType === 3 && !domUtils.isEmptyDom(child))) {
            hasLineChild = true;
        }
        if (hasBlockChild && hasLineChild) {
            break;
        }
    }
    return {
        hasBlock: hasBlockChild,
        hasLine: hasLineChild
    }
}

function patchExecForFormatBlock (blockType) {
    var range = rangeUtils.getRange();
    if (!range) {
        return;
    }
    var start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
    var end = rangeUtils.getRangeDetail(range.endContainer, range.endOffset);
    var prev = domUtils.getPreviousNode(start.container, false) || ENV.doc.body;
    var next = domUtils.getNextNode(end.container, false) || ENV.doc.body;

    var hList = getHBlockList();
    var i, h, b;
    for (i = 0; i < hList.length; i++) {
        h = hList[i];
        if (!domUtils.isTag(h, [blockType])) {
            domUtils.replaceTagName(h, blockType);
        }
    }

    var bList = getBlockList();
    for (i = 0; i < bList.length; i++) {
        b = bList[i];
        domUtils.replaceTagName(b, blockType);
    }

    if (!start.container.parentNode) {
        start = {
            container: prev,
            offset: domUtils.getEndOffset(prev)
        }
    }
    if (!end.container.parentNode) {
        end = {
            container: next,
            offset: 0
        }
    }
    rangeUtils.setRange(start.container, start.offset, end.container, end.offset);
}

/**
 * pC & mac 客户端 使用的 Chrome 版本 49 对于操作 subscript superscript 有 bug 必须修正
 * 如果 sub 或 sup 内的 dom 有 <span> <b> 等标签就会导致无法取消 sub sup
 */
function patchExecForSubAndSup (command) {
    if (patchQueryForSubAndSup(command)) {
        // 格式刷目标第一个元素是目标元素 则清理 sub || sup
        commandExtend.clearSubSup();
        return false;
    } else {
        // 设置 sub || sup
        return ENV.doc.execCommand(command, false);
    }
}

/**
 * pC & mac 客户端 使用的 Chrome 版本 49 对于操作 subscript superscript 有 bug 必须修正
 * 如果 sub 或 sup 内的 dom 有 <span> <b> 等标签就会导致永远得到 false
 */
function patchQueryForSubAndSup (command) {
    var tagName = command === 'subscript' ? 'sub' : 'sup';
    // 设置 sub || sup
    var range = rangeUtils.getRange();
    var start;
    if (range) {
        start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
        start = start.container;
        start = domUtils.getFirstDeepChild(start);
        start = domUtils.getParentByTagName(start, tagName, true);
    }
    return !!start;
}

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.EXEC_COMMEND, _event.handler.onExecCommand);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.EXEC_COMMEND, _event.handler.onExecCommand);
    },
    handler: {
        onExecCommand: function () {
            commandExtend.execCommand.apply(this, arguments);
        }
    }
};

var commandExtend = {
    on: function () {
        _event.bind();
    },
    off: function () {
        _event.unbind();
    },
    clearSubSup: function () {
        var rangeResult = rangeUtils.getRangeDomList({
            noSplit: false
        });
        var i, dom, p;
        var range, span;
        var start, end;
        if (!rangeResult || rangeResult.list.length === 0) {
            range = rangeUtils.getRange();
            if (range && range.collapsed) {
                span = domUtils.createSpan();
                span.innerHTML = CONST.FILL_CHAR + CONST.FILL_CHAR;
                range.insertNode(span);
                p = domUtils.getParentByTagName(span, ['sub', 'sup'], true);
                if (p) {
                    p = domUtils.splitDomSingle(p, span);
                    domUtils.peelDom(p);
                    rangeUtils.setRange(span, 1);
                } else {
                    domUtils.remove(span);
                }
            }
            return;
        }

        var firstP = domUtils.getParentByTagName(rangeResult.list[0], ['sub', 'sup'], true);
        // 只有一个 dom
        if (rangeResult.list.length === 1) {
            p = domUtils.splitDomSingle(firstP, rangeResult.list[0]);
            if (p) {
                start = p.firstChild;
                end = p.lastChild;
                domUtils.peelDom(p);
                rangeUtils.setRange(start, 0, end, domUtils.getEndOffset(end));
            } else {
                rangeUtils.setRange(rangeResult.startDom, rangeResult.startOffset,
                    rangeResult.endDom, rangeResult.endOffset);
            }
            return;
        }

        // 必须要在 剥离 sub sup 之前先让选择范围最前、最后进行分割
        if (firstP) {
            domUtils.splitDomBeforeSub(firstP, rangeResult.list[0]);
        }
        var lastP = domUtils.getParentByTagName(rangeResult.list[rangeResult.list.length - 1], ['sub', 'sup'], true);
        if (lastP) {
            domUtils.splitDomAfterSub(lastP, rangeResult.list[rangeResult.list.length - 1]);
        }

        var isPeelSub = false;
        for (i = 0; i < rangeResult.list.length; i++) {
            dom = rangeResult.list[i];
            p = domUtils.getParentByTagName(dom, ['sub', 'sup'], true);
            if (i === 0) {
                start = dom;
            } else if (i === rangeResult.list.length - 1) {
                end = dom;
            }
            if (p) {
                if (i === 0) {
                    start = p.firstChild;
                } else if (i === rangeResult.list.length - 1) {
                    end = p.lastChild;
                }
                domUtils.peelDom(p);
                isPeelSub = true;
            }
        }
        // 操作后必须修正 Range
        if (isPeelSub) {
            rangeUtils.setRange(start, 0, end, domUtils.getEndOffset(end));
        } else {
            rangeUtils.fixRange(rangeResult);
        }
    },
    /**
     * 封装 execCommand 方法，用于 编辑器内部事件触发
     * @returns {*}
     */
    execCommand: function () {
        var range = rangeUtils.getRange();
        var startTodoMain, startTodoCheckbox, startTodoId;
        if (range && range.collapsed) {
            startTodoMain = todoUtils.getMainByCaret();
            startTodoCheckbox = todoUtils.getCheckbox(startTodoMain);
            startTodoId = startTodoCheckbox ? startTodoCheckbox.id : '';
        }

        var result;
        if (/formatblock/i.test(arguments[0])) {
            result = patchExecForFormatBlock(arguments[2]);
        } else if (/subscript|superscript/i.test(arguments[0])) {
            result = patchExecForSubAndSup(arguments[0]);
        } else {
            result = ENV.doc.execCommand.apply(ENV.doc, arguments);
        }

        var command = arguments[0];

        // fix orderList className
        if (/^(indent)|(outdent)|(insertOrderedList)|(insertUnorderedList)$/i.test(command)) {
            domUtils.fixOrderList();
        }

        // patch for ios
        // 修正 ios 客户端出现 的 bug：
        // 无序列表 + todoList 文本，在行末尾回车，然后立刻执行 缩进操作，然后输入文本，文本会被加入 font 标签而且会设置小一号的字体
        // 解决办法，缩进后判断如果是空的 todoList，则将checkbox 后内容删除重新设置 span 标签
        if (startTodoId && ((/^indent$/i.test(command) && ENV.client.type.isIOS) ||
            /^outdent$/i.test(command))) {

            if (/^outdent$/i.test(command)) {
                // 减少缩进操作会导致将 todoList 的 container 剥离
                todoUtils.fixNewTodo();
            }
            startTodoCheckbox = ENV.doc.querySelector('#' + startTodoId);
            startTodoMain = todoUtils.getMainFromChild(startTodoCheckbox);

            if (startTodoMain && todoUtils.isEmptyMain(startTodoMain)) {
                var checkbox = todoUtils.getCheckbox(startTodoMain);
                while (startTodoMain.lastChild !== checkbox) {
                    startTodoMain.removeChild(startTodoMain.lastChild);
                }
                var span = domUtils.createSpan();
                domUtils.after(span, checkbox);
                span.innerHTML = CONST.FILL_CHAR + CONST.FILL_CHAR;
                rangeUtils.setRange(span, 1);
            }
        }

        // PC 客户端使用的 Chrome 引擎在 设置列表后，会自动出现 font-size 的样式，必须要进行修正
        domUtils.fixFontSize();

        ENV.event.call(CONST.EVENT.ON_EXEC_COMMAND);
        return result;
    },
    queryCommandState: function (command) {
        var result;
        if (/subscript|superscript/i.test(command)) {
            result = patchQueryForSubAndSup(command);
        } else {
            result = ENV.doc.queryCommandState(command);
        }
        return result;
    }

};

module.exports = commandExtend;

},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"../todoUtils/todoUtils":59}],34:[function(require,module,exports){
/**
 * editor 使用的基本事件处理
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    Lang = require('../common/lang'),
    LANG = Lang.getLang(),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    domUtils = require('../domUtils/domExtend'),
    codeUtils = require('../codeUtils/codeUtils'),
    formatPainterCore = require('../formatPainterUtils/formatPainterCore'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    tableCore = require('../tableUtils/tableCore'),
    tableZone = require('../tableUtils/tableZone'),
    tableUtils = require('../tableUtils/tableUtils'),
    todoUtils = require('../todoUtils/todoUtils'),
    amend = require('../amend/amend'),
    amendUser = require('../amend/amendUser'),
    amendUtils = require('../amend/amendUtils/amendExtend'),
    pasteUtils = require('./pasteUtils'),
    commandExtend = require('./commandExtend');

var EditorEventType = {
    SelectionChange: 'selectionchange'
}, editorListener = {
    selectionchange: []
};

var eventTrackHandler = {},
    selectTimer = null;

/**
 * 获取光标位置 文字样式
 */
function getCaretStyle() {
    if (selectTimer) {
        clearTimeout(selectTimer);
    }

    var range = rangeUtils.getRange(),
        zone = tableZone.getZone();

    if ((!range && !zone.range) || zone.active) {
        return;
    }
    selectTimer = setTimeout(_getCaretStyle, 200);
}
function _getCaretStyle() {
    var result = {
        'backColor': '',
        'blockFormat': '',
        'bold': '0',
        'canCreateCode': '1',
        'canCreateTable': '1',
        'canCreateTodo': '1',
        'clientTools': '1',
        'fontName': '',
        'fontSize': '',
        'foreColor': '',
        'formatPainterStatus': formatPainterCore.status().toString(),
        'InsertOrderedList': '0',
        'InsertUnorderedList': '0',
        'italic': '0',
        'justifyleft': '0',
        'justifycenter': '0',
        'justifyright': '0',
        'justifyfull': '0',
        'strikeThrough': '0',
        'subscript': '0',
        'superscript': '0',
        'underline': '0'
    }, style;
    var range = rangeUtils.getRange(),
        zone = tableZone.getZone(),
        cells, cellsAlign,
        rangeList = [];

    if (!range && (!zone.range || zone.active)) {
        return;
    }

    if (zone.grid && zone.range) {
        // 在表格内
        result.canCreateTable = '0';
        result.canCreateCode = '0';
    } else if (range && codeUtils.getContainerFromChild(range.startContainer)) {
        // 在 CodeMirror 内
        result.clientTools = '0';
        result.canCreateCode = '0';
        result.canCreateTable = '0';
        result.canCreateTodo = '0';
        result.canFormatPainter = '0';
    } else if (!todoUtils.canCreateTodo()) {
        result.canCreateTodo = '0';
    }

    if (range && (!zone.range || tableZone.isSingleCell())) {
        result.blockFormat = ENV.doc.queryCommandValue("formatBlock");
        result.fontName = ENV.doc.queryCommandValue("fontName");
        result.foreColor = utils.rgb2Hex(ENV.doc.queryCommandValue('foreColor'));
        result.backColor = utils.rgb2Hex(ENV.doc.queryCommandValue('backColor'));
        result.bold = queryCommand('bold');
        result.italic = queryCommand('italic');
        result.underline = queryCommand('underline');
        result.strikeThrough = queryCommand('strikeThrough');
        result.subscript = queryCommand('subscript');
        result.superscript = queryCommand('superscript');
        result.justifyleft = queryCommand('justifyleft');
        result.justifycenter = queryCommand('justifycenter');
        result.justifyright = queryCommand('justifyright');
        result.justifyfull = queryCommand('justifyfull');
        result.InsertOrderedList = queryCommand('InsertOrderedList');
        result.InsertUnorderedList = queryCommand('InsertUnorderedList');

        style = {
            'font-size': ''
        };

        rangeList = rangeUtils.getRangeDomList({
            noSplit: true
        });
        if (rangeList) {
            rangeList = rangeList.list.length > 0 ? rangeList.list : [rangeList.startDom];
        }

    } else {
        // 表格 多选单元格的时候
        cellsAlign = tableUtils.getAlign(zone.grid, zone.range);
        cells = tableZone.getSelectedCells();
        rangeList = tableUtils.getDomsByCellList(cells);

        result.justifyleft = cellsAlign.align === 'left' ? '1' : '0';
        result.justifycenter = cellsAlign.align === 'center' ? '1' : '0';
        result.justifyright = cellsAlign.align === 'right' ? '1' : '0';

        style = {
            'font-size': '',
            'font-family': '',
            'font-weight': '',
            'font-style': '',
            'text-decoration': '',
            'color': '',
            'background-color': ''
        };
    }

    var i, j, k, v, o;
    for (i = 0, j = rangeList.length; i < j; i++) {
        o = rangeList[i];
        if (o.nodeType !== 3 && o.nodeType !== 1) {
            continue;
        }
        o = o.nodeType === 3 ? o.parentNode : rangeList[i];
        for (k in style) {
            if (style.hasOwnProperty(k)) {
                v = domUtils.getComputedStyle(o, k, true);
                if (!v) {
                    continue;
                }

                if (i === 0) {
                    style[k] = v;
                } else if (style[k] !== v) {
                    style[k] = '';
                }
            }
        }
    }

    var s = style['font-size'];
    if (s) {
        result['fontSize'] = Math.round(parseFloat(s) / domUtils.pt2px() / domUtils.getRootSizeRate()) + 'pt';
    }
    s = style['font-family'];
    if (s) {
        result['fontName'] = s;
    }
    s = style['font-weight'];
    if (s && /bold|bolder/.test(s)) {
        result['bold'] = '1';
    }
    s = style['font-style'];
    if (s && /italic|oblique/.test(s)) {
        result['italic'] = '1';
    }
    s = style['text-decoration'];
    if (s && /underline/.test(s)) {
        result['underline'] = '1';
    } else if (s && /line\-through/.test(s)) {
        result['strikeThrough'] = '1';
    }

    //默认的选中节点背景色 需要过滤掉
    if (result.backColor === '#f3f7ff') {
        result.backColor = '';
    }
    // console.log(result.fontSize);
    EditorEvent.triggerListener(EditorEventType.SelectionChange, [result]);

    function queryCommand(command) {
        return commandExtend.queryCommandState(command) ? "1" : "0";
    }
}

/**
 * 复制/剪切 选中区域
 * @param e
 * @param isCut
 */
function copySelection(e, isCut) {
    var zone = tableZone.getZone(),
        range = rangeUtils.getRange(),
        fragment, oldHtml, newHtml,
        hasCodeMirror = false,
        tmpParent, tmp,
        canSetData = true,
        user, style, domList = [];
    isCut = !!isCut;

    // 检查复制的区域内是否存在 codeMirror
    if (!zone.range && range && !range.collapsed) {
        tmp = range.cloneContents();
        hasCodeMirror = !!tmp.querySelector('.' + CONST.CLASS.CODE_CONTAINER);
    }

    //只有在修订状态时才禁止复制 已删除 的内容
    if (range) {
      tmpParent = domUtils.getParentRoot([range.startContainer, range.endContainer]);
      if (amend.isAmendEditing() && tmpParent &&
        tmpParent.getAttribute(CONST.ATTR.SPAN_DELETE) === amendUser.getCurUser().hash) {
        alert(isCut ? LANG.Err.Cut_Null : LANG.Err.Copy_Null);
        utils.stopEvent(e);
        return;
      }
    }


    // 只有选中 表格、修订、CodeMirror 区域 才干涉选中内容
    if ((!zone.range && (!range || range.collapsed || (!hasCodeMirror && !amend.isAmendEditing()))) ||
        (zone.range && range && !range.collapsed)) {
        return;
    }

    if (zone.range && (!range || range.collapsed)) {
        fragment = tableZone.getFragmentForCopy();
    } else if (range && !range.collapsed) {
        fragment = rangeUtils.getFragmentForCopy();
    }
    if (fragment) {
        if (amend.isAmendEditing()) {
            oldHtml = fragment.innerHTML.length;
            amend.fragmentFilter(fragment);
            newHtml = fragment.innerHTML.length;
            if (newHtml === 0 && oldHtml > 0) {
                alert(LANG.Err.Copy_Null);
                canSetData = false;
            }
        }

        if (canSetData) {
            e.clipboardData.clearData();
            e.clipboardData.setData('text/plain', fragment.innerText);
            e.clipboardData.setData('text/html', fragment.innerHTML);

            if (isCut) {
                historyUtils.saveSnap(false);
            }

            if (isCut && amend.isAmendEditing()) {
                user = amendUser.getCurUser();
                style = amendUtils.getDeletedStyle(user);
                if (zone.range && (!range || range.collapsed)) {
                    //处理表格内部时，必须要把每一个叶子节点找出来，然后去处理
                    domList = tableUtils.getDomsByCellList(tableZone.getSelectedCells());

                    rangeUtils.modifyDomsStyle(domList, style.style, style.attr, []);
                    amendUtils.removeUserDel(zone.table, user);
                } else {

                    amendUtils.removeSelection(user);
                    amendUtils.removeUserDel(null, user);
                    ENV.doc.getSelection().collapseToEnd();
                    rangeUtils.caretFocus();
                }
            } else if (isCut && zone.range && (!range || range.collapsed)) {
                tableCore.clearCellValue();
            }
        }

        fragment.innerHTML = '';
        if (fragment.parentNode) {
          fragment.parentNode.removeChild(fragment);
        }
        fragment = null;

    }

    utils.stopEvent(e);
}

function eventStringify(event) {
    var k, v, t, s = [];
    for (k in event) {
        v = event[k];
        t = (typeof v).toLowerCase();
        if (t === 'string' || t === 'number') {
            if (t === 'string') {
                v = '"' + v.replace(/"/g, '\\"') + '"';
            }
            s.push('"' + k + '":' + v);
        }
    }
    return '{' + s.join(',') + '}';
}

var EditorEvent = {
    TYPE: EditorEventType,
    bind: function () {
        EditorEvent.unbind();
        ENV.doc.addEventListener('click', handler.onClick);
        ENV.doc.addEventListener('compositionstart', handler.onCompositionstart);
        ENV.doc.addEventListener('compositionend', handler.onCompositionend);
        ENV.doc.addEventListener('copy', handler.onCopy);
        ENV.doc.addEventListener('cut', handler.onCut);
        ENV.doc.addEventListener('DOMSubtreeModified', handler.onDOMSubtreeModified);
        ENV.doc.addEventListener('dragstart', handler.onDragStart);
        ENV.doc.addEventListener('dragenter', handler.onDragEnter);
        ENV.doc.addEventListener('drop', handler.onDrop);
        ENV.doc.addEventListener('keydown', handler.onKeydown);
        ENV.doc.addEventListener('keyup', handler.onKeyup);
        ENV.doc.addEventListener('mousedown', handler.onMousedown);
        ENV.doc.addEventListener('mousemove', handler.onMousemove);
        ENV.doc.addEventListener('mouseover', handler.onMouseover);
        ENV.doc.addEventListener('mouseup', handler.onMouseup);
        ENV.doc.addEventListener('orientationchange', handler.onOrientationchange);
        ENV.doc.addEventListener('paste', handler.onPaste);
        ENV.doc.addEventListener('scroll', handler.onScroll);
        ENV.doc.addEventListener('selectstart', handler.onSelectionStart);
        ENV.doc.addEventListener('selectionchange', handler.onSelectionChange);

        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            ENV.doc.addEventListener('touchend', handler.onTouchEnd);
            ENV.doc.addEventListener('touchstart', handler.onTouchStart);
        }
    },
    unbind: function () {
        ENV.doc.removeEventListener('click', handler.onClick);
        ENV.doc.removeEventListener('compositionstart', handler.onCompositionstart);
        ENV.doc.removeEventListener('compositionend', handler.onCompositionend);
        ENV.doc.removeEventListener('copy', handler.onCopy);
        ENV.doc.removeEventListener('cut', handler.onCut);
        ENV.doc.removeEventListener('dragstart', handler.onDragStart);
        ENV.doc.removeEventListener('dragenter', handler.onDragEnter);
        ENV.doc.removeEventListener('drop', handler.onDrop);
        ENV.doc.removeEventListener('keydown', handler.onKeydown);
        ENV.doc.removeEventListener('keyup', handler.onKeyup);
        ENV.doc.removeEventListener('mousedown', handler.onMousedown);
        ENV.doc.removeEventListener('mousemove', handler.onMousemove);
        ENV.doc.removeEventListener('mouseover', handler.onMouseover);
        ENV.doc.removeEventListener('mouseup', handler.onMouseup);
        ENV.doc.removeEventListener('orientationchange', handler.onOrientationchange);
        ENV.doc.removeEventListener('paste', handler.onPaste);
        ENV.doc.removeEventListener('scroll', handler.onScroll);
        ENV.doc.removeEventListener('selectstart', handler.onSelectionStart);
        ENV.doc.removeEventListener('selectionchange', handler.onSelectionChange);
        ENV.doc.removeEventListener('touchend', handler.onTouchEnd);
        ENV.doc.removeEventListener('touchstart', handler.onTouchStart);
    },
    startTrackEvent: function (eventName, id) {
        if (!eventTrackHandler[id]) {
            eventTrackHandler[id] = function (event) {
                ENV.client.sendCmdToWiznote(CONST.CLIENT_EVENT.WizEditorTrackEvent, {
                    id: id,
                    e: eventStringify(event)
                });
            };
            ENV.doc.addEventListener(eventName, eventTrackHandler[id]);
        }
    },
    stopTrackEvent: function (eventName, id) {
        if (eventTrackHandler[id]) {
            ENV.doc.removeEventListener(eventName, eventTrackHandler[id]);
            delete eventTrackHandler[id];
        }
    },
    addListener: function (eName, fun) {
        if (!fun || typeof fun !== 'function') {
            return;
        }
        var h = editorListener[eName];
        if (!h) {
            return;
        }
        var i, j;
        for (i = 0, j = h.length; i < j; i++) {
            if (h[i] === fun) {
                return;
            }
        }
        h.push(fun);
        if (eName === EditorEventType.SelectionChange) {
            ENV.event.add(CONST.EVENT.ON_SELECTION_CHANGE, getCaretStyle);
        }
    },
    removeListener: function (eName, fun) {
        if (fun && typeof fun !== 'function') {
            return;
        }
        var h = editorListener[eName];
        if (!h) {
            return;
        }
        var i;
        for (i = h.length - 1; i >= 0; i--) {
            if (h[i] === fun || !fun) {
                h.splice(i, 1);
            }
        }

        if (h.length === 0) {
            ENV.event.remove(CONST.EVENT.ON_SELECTION_CHANGE, getCaretStyle);
        }
    },
    triggerListener: function (eName, params) {
        var h = editorListener[eName];
        if (!h) {
            return;
        }
        var i, j, f;
        for (i = 0, j = h.length; i < j; i++) {
            f = h[i];
            f.apply(this, params);
        }
    }
};

var handler = {
    onClick: function (e) {
        ENV.event.call(CONST.EVENT.ON_CLICK, e);
    },
    onCompositionstart: function (e) {
        ENV.event.call(CONST.EVENT.ON_COMPOSITION_START, e);
    },
    onCompositionend: function (e) {
        ENV.event.call(CONST.EVENT.ON_COMPOSITION_END, e);
    },
    onCopy: function (e) {
        // CodeMirror 内 复制操作全部忽略
        if (codeUtils.getContainerFromChild(e.target)) {
            return;
        }
        copySelection(e, false);
        // ENV.event.call(CONST.EVENT.ON_COPY, e);
    },
    onCut: function (e) {
        // CodeMirror 内 剪贴操作全部忽略
        if (codeUtils.getContainerFromChild(e.target)) {
            return;
        }
        historyUtils.saveSnap(false);
        copySelection(e, true);
        ENV.event.call(CONST.EVENT.ON_CUT, e);
    },
    onDOMSubtreeModified: function (e) {
        ENV.event.call(CONST.EVENT.ON_DOM_SUBTREE_MODIFIED, e);
    },
    onDragStart: function (e) {
        ENV.event.call(CONST.EVENT.ON_DRAG_START, e);
    },
    onDragEnter: function (e) {
        ENV.event.call(CONST.EVENT.ON_DRAG_ENTER, e);
    },
    onDrop: function (e) {
        ENV.event.call(CONST.EVENT.ON_DROP, e);
    },
    onKeydown: function (e) {
        // 修正 body 避免 body 为空
        var range = rangeUtils.getRange();
        var line;
        if (range && range.collapsed && ENV.doc.body && ENV.doc.body.childNodes.length === 0) {
            line = ENV.doc.createElement('div');
            line.appendChild(ENV.doc.createElement('br'));
            ENV.doc.body.appendChild(line);
            rangeUtils.setRange(line, 1);
        }

        var keyCode = e.keyCode || e.which;

        if (codeUtils.getContainerFromChild(e.target)) {
            // CodeMirror 内键盘操作全部忽略
            return;
        }

        // Mac 端 Ctrl + S 触发保存操作
        if (ENV.win.WizQtEditor && ENV.client.type.isMac && keyCode === 83 &&
            ((ENV.client.type.isLinux && e.ctrlKey) || (!ENV.client.type.isLinux && e.metaKey))) {
            ENV.win.WizQtEditor.saveCurrentNote();
        }

        var sel, method, direct, type;
        var zone = tableZone.getZone();
        // Mac 端快捷键
        if (ENV.win.WizQtEditor && ENV.client.type.isMac && !zone.range) {
            sel = ENV.doc.getSelection();
            method = e.shiftKey ? 'extend' : 'move';
            if ((e.metaKey && keyCode == 37) ||
                (e.ctrlKey && keyCode == 65)) {
                // Cmd + left or  Ctrl + A
                direct = 'backward';
                type = 'lineboundary';
            } else if ((e.metaKey && keyCode == 39) ||
                (e.ctrlKey && keyCode == 69)) {
                // Cmd + right or  Ctrl + E
                direct = 'forward';
                type = 'lineboundary';
            } else if (e.metaKey && keyCode == 38) {
                // Cmd + up
                direct = 'backward';
                type = 'documentboundary';
            } else if (e.metaKey && keyCode == 40) {
                // Cmd + down
                direct = 'forward';
                type = 'documentboundary';
            } else if (e.ctrlKey && keyCode == 80) {
                // Ctrl + P
                direct = 'backward';
                type = 'line';
            } else if (e.ctrlKey && keyCode == 78) {
                // Ctrl + N
                direct = 'forward';
                type = 'line';
            } else if (e.ctrlKey && keyCode == 70) {
                // Ctrl + F
                direct = 'forward';
                type = 'character';
            } else if (e.ctrlKey && keyCode == 66) {
                // Ctrl + B
                direct = 'backward';
                type = 'character';
            }
        }

        if (direct) {
            // 满足快捷键需求，则修改 range
            sel.modify(method, direct, type);
            rangeUtils.fixScroll();
            return;
        }

        ENV.event.call(CONST.EVENT.ON_KEY_DOWN, e);
    },
    onKeyup: function (e) {
        ENV.event.call(CONST.EVENT.ON_KEY_UP, e);
        // IOS 在键盘输入时，需要修正光标位置，保证光标进入可视区
        if (ENV.client.type.isIOS) {
            rangeUtils.fixScroll();
        }
    },
    onMousedown: function (e) {
        ENV.event.call(CONST.EVENT.ON_MOUSE_DOWN, e);
    },
    onMousemove: function (e) {
        ENV.event.call(CONST.EVENT.ON_MOUSE_MOVE, e);
    },
    onMouseover: function (e) {
        ENV.event.call(CONST.EVENT.ON_MOUSE_OVER, e);
    },
    onMouseup: function (e) {
        ENV.event.call(CONST.EVENT.ON_MOUSE_UP, e);
    },
    onOrientationchange: function (e) {
        ENV.event.call(CONST.EVENT.ON_ORIENTATION_CHANGE, e);
        // IOS 横竖屏转换后 自动修正光标进入可视区
        if (ENV.client.type.isIOS) {
            rangeUtils.fixScroll();
        }
    },
    onPaste: function (e) {
        if (pasteUtils.pasteFromClipBoard(e)) {
            ENV.event.call(CONST.EVENT.ON_PASTE, e);
        }
    },
    onScroll: function (e) {
        ENV.event.call(CONST.EVENT.ON_SCROLL, e);
    },
    onSelectionStart: function (e) {
        ENV.event.call(CONST.EVENT.ON_SELECT_START, e);
    },
    onSelectionChange: function (e) {
        ENV.event.call(CONST.EVENT.ON_SELECTION_CHANGE, e);
        // IOS 在键盘输入时，需要修正光标位置，保证光标进入可视区
        if (ENV.client.type.isIOS) {
            rangeUtils.fixScroll();
        }
        // console.log(JSON.stringify(rangeUtils.getRange().getClientRects()));
        // console.log(rangeUtils.getRange().getClientRects());
    },
    onTouchEnd: function (e) {
        ENV.event.call(CONST.EVENT.ON_TOUCH_END, e);
    },
    onTouchStart: function (e) {
        ENV.event.call(CONST.EVENT.ON_TOUCH_START, e);
    }
};

module.exports = EditorEvent;
},{"../amend/amend":7,"../amend/amendUser":9,"../amend/amendUtils/amendExtend":11,"../codeUtils/codeUtils":16,"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/lang":22,"../common/utils":24,"../domUtils/domExtend":29,"../formatPainterUtils/formatPainterCore":37,"../rangeUtils/rangeExtend":49,"../tableUtils/tableCore":52,"../tableUtils/tableUtils":54,"../tableUtils/tableZone":55,"../todoUtils/todoUtils":59,"./commandExtend":33,"./pasteUtils":35}],35:[function(require,module,exports){
/**
 * paste 粘贴操作基本库
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    domUtils = require('../domUtils/domExtend'),
    xssUtils = require('../domUtils/xssUtils'),
    codeUtils = require('../codeUtils/codeUtils'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    tableCore = require('../tableUtils/tableCore'),
    tableZone = require('../tableUtils/tableZone'),
    tableUtils = require('../tableUtils/tableUtils'),
    amend = require('../amend/amend'),
    amendUser = require('../amend/amendUser'),
    amendUtils = require('../amend/amendUtils/amendExtend'),
    commandExtend = require('./commandExtend');

/**
 * 从剪切板粘贴内容
 * @param e
 */
function clearClipHtml (html) {
    // excel 复制时， </html>后面有乱码，需要过滤
    // <html> <body> 之间会有 换行符，也必须要过滤
    // var startTag = ['<!--StartFragment-->', '<body>', '<html>'];
    // var endTag = ['<!--EndFragment-->', '</body>', '</html>'];

    // 不能只保留 StartFragment 内的东西， Excel 复制的表格 <table> 会处于外面
    // var fragmentReg = /<!--StartFragment-->((\r?\n|.)*)<!--EndFragment-->/gi;
    var fragmentReg = /<body[^>]*>((\r?\n|.)*)(\r?\n)*<\/body>/gi;
    var headReg = /<head[^>]*>((\r?\n|.)*)(\r?\n)*<\/head>/i;
    var styleReg = /<style[^>]*>[^<>]*<\/style>/ig;
    var body = fragmentReg.exec(html), head, m;
    if (body) {
        // xcode 复制的内容是标准的 html 页面， style 会存在于 head 内
        head = headReg.exec(html);
        html = '';
        if (head) {
            head = head[1];
            head = head.replace(/<!--.*?(-->)/g, '');
            while ((m = styleReg.exec(head)) !== null) {
                if (m.index === styleReg.lastIndex) {
                    styleReg.lastIndex++;
                }
                html += m[0];
            }
        }
        html += body[1];
    } else {
        html = html.replace(/<!--.*?(-->)/g, '');
        var startTag = [/<body( [^<>]*)?>/i, /<html( [^<>]*)?>/i];
        var endTag = [/<\/body>/i, /<\/html>/i];
        var result, reg, i, j;

        for (i = 0, j = startTag.length; i < j; i++) {
            reg = startTag[i];
            result = html.match(reg);
            if (result) {
                html = html.substr(result.index + result[0].length);
                break;
            }
        }
        for (i = 0, j = endTag.length; i < j; i++) {
            reg = endTag[i];
            result = html.match(reg);
            if (result) {
                html = html.substr(0, result.index);
            }
        }
    }
    // 清理前后换行
    html = html.replace(/(^(\r?\n)*)|((\r?\n)*$)/g, '');

    // 将文本内的 连续空格 替换为 &nbsp;
    html = html.replace(/\u00A0/ig, '&nbsp;');

    html = domUtils.clearStyleFromHtml(html,
        ['display', 'color', 'width',
            'font-family', 'font-size', 'font-style', 'font-weight',
            'text-align', 'text-indent',
            'background*', 'text-decoration*', 'padding*', 'margin*', 'list*', 'border*']);
    html = xssUtils.xssFilter(html);
    return html;
}

function txt2Dom (txt) {
    var regTab = /	/g;
    var regSpace = /  /g;
    txt = (txt || '').replace(regTab, '    ').replace(regSpace, '\u00A0 ');
    var result = [];
    var lineList = txt.split(/\r?\n/);
    var lineStr, i, line;
    for (i = 0; i < lineList.length; i++) {
        lineStr = lineList[i];
        line = lineStr ? ENV.doc.createTextNode(lineStr) : ENV.doc.createElement('br');
        result.push(line);
    }
    return result;
}

function insertBr () {
    var sel = ENV.doc.getSelection();
    var range = rangeUtils.getRange();
    var start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
    var line = domUtils.getParentByTagName(start.container, ['li', 'td', 'th'], true);
    var span;
    if (domUtils.isEmptyDom(line)) {
        // 对于 列表内必须特殊处理
        span = ENV.doc.createElement('span');
        span.innerHTML = CONST.FILL_CHAR;
        rangeUtils.setRange(line, 0, line, domUtils.getEndOffset(line));
        range = rangeUtils.getRange();
        range.deleteContents();
        range.insertNode(span);
        rangeUtils.setRangeToEnd(span);
        commandExtend.execCommand('insertparagraph');
        line.innerHTML = '<br>';

    } else {
        commandExtend.execCommand('insertparagraph');
    }
}

function insertTxt (txt) {
    var container = ENV.doc.createElement('div');
    var domList = txt2Dom(txt);
    var sel = ENV.doc.getSelection();
    var range, i;
    var isFirstLine = true, start, line, dom;

    for (i = 0; i < domList.length; i++) {
        container.appendChild(domList[i]);
    }

    while (container.firstChild) {
        dom = container.firstChild;
        if (!isFirstLine) {
            insertBr();
        }
        range = rangeUtils.getRange();
        if (domUtils.isTag(dom, 'br')) {
            container.removeChild(dom);
        } else {
            start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            line = domUtils.getParentByTagName(start.container, ['li', 'td', 'th'], true);
            if (domUtils.isEmptyDom(line)) {
                rangeUtils.setRange(line, 0, line, domUtils.getEndOffset(line));
            }
            range = rangeUtils.getRange();
            range.deleteContents();
            range.insertNode(dom);
            rangeUtils.setRangeToEnd(dom);
        }
        sel.collapseToEnd();
        isFirstLine = false;
    }
}

function pasteWithStandard (html, txt) {
    var container = ENV.doc.createElement('div');
    var fixed;

    if (amend.isAmendEditing()) {
        //修订模式下， 表格外 粘贴
        amend.readyForPaste();
    } else {
        //非修订模式下， 表格外 粘贴
        fixed = amendUtils.fixedAmendRange();
        amend.splitAmendDomByRange(fixed);
    }
    var sel = ENV.doc.getSelection();
    var range = rangeUtils.getRange();
    var start, line;
    var tmpParent, tmpChild, pasteTag, targetBlock;
    var lastTag = null;

    if (html) {
        container.innerHTML = html;

        // 修正复制过来的 CodeMirror
        codeUtils.pastePatch.ready(container);

        // 粘贴起始位置 是空行，则选中行内所有元素（主要用于减少 br）
        start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
        line = domUtils.getParentByTagName(start.container, ['li', 'td', 'th'], true);
        if (domUtils.isEmptyDom(line)) {
            rangeUtils.setRange(line, 0, line, domUtils.getEndOffset(line));
            range = rangeUtils.getRange();
        }

        while (container.firstChild) {
            range = rangeUtils.getRange();
            start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            targetBlock = domUtils.getBlockParent(start.container, true);
            tmpParent = domUtils.getParentByTagName(targetBlock, ['pre'], true);
            if (tmpParent) {
                targetBlock = tmpParent;
            }
            pasteTag = container.firstChild;
            if (/^h[1-6]$/i.test(targetBlock.tagName) &&
                (pasteTag.tagName === targetBlock.tagName ||
                pasteTag.nodeType === 3 ||
                domUtils.isTag(pasteTag, ['span']))) {
                // 如果 光标位置是 H1 - H6 且 粘贴的元素 是同样的 H1 - H6 或者 是 span 和 textNode
                // 则直接当作文本接入 H1 - H6
                tmpChild = ENV.doc.createElement('span');
                tmpChild.appendChild(ENV.doc.createTextNode(pasteTag.nodeType === 3 ? pasteTag.nodeValue : pasteTag.innerText));
                range.deleteContents();
                range.insertNode(tmpChild);
                container.removeChild(pasteTag);
                pasteTag = tmpChild;

            } else if (/^h[1-6]$/i.test(targetBlock.tagName) ||
                (/^h[1-6]$/i.test(pasteTag.tagName) && targetBlock !== ENV.doc.body)) {
                // 如果 光标位置 与 粘贴的元素 是不相同的 H1 - H6，
                // 或 光标位置不是 H1 - H6 且 粘贴的元素 是 H1 - H6，
                // 或 光标位置是 H1 - H6 且 粘贴的元素 是普通块元素
                // 则从光标位置拆分 光标位置的 Block，然后插入粘贴的元素
                tmpChild = ENV.doc.createElement('span');
                range.insertNode(tmpChild);
                targetBlock = domUtils.splitDomBeforeSub(targetBlock, tmpChild);
                domUtils.after(pasteTag, targetBlock);
                if (domUtils.isEmptyDom(targetBlock)) {
                    domUtils.remove(targetBlock);
                } else {
                    domUtils.remove(tmpChild);
                }
            } else if (lastTag) {
                // 当不满足以上条件，且是粘贴中非首个元素时，直接将 粘贴的元素放置到 last 后面
                domUtils.after(pasteTag, lastTag);
            } else {
                // 其他情况全部使用 insertNode 方法让浏览器自行处理
                range.insertNode(pasteTag);
            }
            rangeUtils.setRangeToEnd(pasteTag);
            // sel.collapseToEnd();
            lastTag = pasteTag;
        }
    } else {
        insertTxt(txt);
    }
    codeUtils.pastePatch.fix();
}

function pasteWithTable (html, txt) {
    // var fixed,
    var template,
        target,
        gridPaste,
        pasteCell, pasteColCount, pasteRowCount, addColCount, addRowCount, maxCol, maxRow,
        x, y, cell;

    var range = rangeUtils.getRange(),
        zone = tableZone.getZone();

    // 在表格内粘贴
    if (html) {
        template = tableUtils.getTemplateByHtmlForPaste(html);
        template.isHtml = true;
        // 修正复制过来的 CodeMirror
        codeUtils.pastePatch.ready(template.pasteDom, true);
    } else if (txt && !tableZone.isSingleCell()) {
        template = tableUtils.getTemplateByTxtForPaste(txt);
        template.isHtml = true;
    } else {
        template = {
            isTable: false,
            isHtml: false,
            pasteDom: ENV.doc.createElement('div')
        };
    }
    // console.log(template.isTable);
    // console.log(template.pasteDom);

    if (!template.isTable) {
        // 粘贴普通文本
        if (!range && zone.range) {
            //如果选择了多个单元格，则只粘贴到左上角的单元格内
            target = zone.grid[zone.range.minY][zone.range.minX].cell;
            tableZone.setStart(target).setEnd(target);
            rangeUtils.setRange(target, 0, target.lastChild, domUtils.getEndOffset(target.lastChild));
        }
        pasteWithStandard(html, txt);
        //
        // // 粘贴 非表格的 普通文本
        // if (amend.isAmendEditing()) {
        //     //修订模式 预处理
        //     amend.readyForPaste();
        // } else {
        //     //非修订模式 预处理
        //     fixed = amendUtils.fixedAmendRange();
        //     amend.splitAmendDomByRange(fixed);
        // }
        //
        // if (!template.isHtml) {
        //     insertTxt(txt);
        //     return;
        // }
        //
        // var tmpTarget, insertBefore, last;
        // range = rangeUtils.getRange();
        // if (range) {
        //     if (range.startContainer.nodeType === 3 &&
        //         range.startOffset > 0 && range.startOffset < range.startContainer.nodeValue.length) {
        //         //如果不符合预处理的条件，并且还处于 TextNode 中间时，需要拆分
        //         target = domUtils.splitRangeText(range.startContainer, range.startOffset, range.startOffset, false);
        //         tmpTarget = target;
        //         insertBefore = false;
        //     } else {
        //         target = range.startContainer;
        //         if (target.nodeType === 3) {
        //             insertBefore = (range.startOffset === 0);
        //         } else if (range.startOffset > 0 && !domUtils.isEmptyDom(target)) {
        //             target = target.childNodes[range.startOffset - 1];
        //             insertBefore = false;
        //         } else {
        //             insertBefore = true;
        //         }
        //         if (domUtils.isTag(target, ['td', 'th']) && domUtils.isEmptyDom(target)) {
        //             //如果 target 是 td 则必须在 td内建立 span，避免插入到 td 后面
        //             target.innerHTML = '';
        //             target.appendChild(domUtils.createSpan());
        //             target = target.childNodes[0];
        //             insertBefore = false;
        //         }
        //     }
        //     last = template.pasteDom.lastChild;
        //     if (insertBefore) {
        //         while (template.pasteDom.firstChild && target) {
        //             domUtils.before(template.pasteDom.firstChild, target, false);
        //         }
        //     } else {
        //         while (template.pasteDom.lastChild && target) {
        //             domUtils.before(template.pasteDom.lastChild, target, true);
        //         }
        //     }
        //
        //     if (tmpTarget) {
        //          domUtils.remove(tmpTarget);
        //     }
        //     rangeUtils.setRange(last, domUtils.getEndOffset(last));

    } else {
        // 粘贴表格

        //分析剪切板内的表格范围
        gridPaste = tableUtils.getTableGrid(template.pasteDom);
        pasteRowCount = gridPaste.length;
        pasteColCount = gridPaste[0] ? gridPaste[0].length : 0;

        //从起始点 cellData 根据 剪切板内表格范围 判断是否需要增加 表格的列、行
        addRowCount = zone.grid.length - zone.range.minY - pasteRowCount;
        addColCount = zone.grid[0].length - zone.range.minX - pasteColCount;

        for (y = addRowCount; y < 0; y++) {
            tableCore.insertRow(false);
        }
        for (x = addColCount; x < 0; x++) {
            tableCore.insertCol(false);
        }

        //分析已选择的表格范围
        zone = tableZone.getZone();
        if (!html) {
            //从文本转义的 table 只复制一次，不允许反复被粘贴
            maxRow = zone.range.minY + pasteRowCount - 1;
            maxCol = zone.range.minX + pasteColCount - 1;
        } else {
            maxRow = Math.max(zone.range.minY + pasteRowCount - 1, zone.range.maxY);
            maxCol = Math.max(zone.range.minX + pasteColCount - 1, zone.range.maxX);
        }

        //从起始点 cellData 开始 循环粘贴剪切板单元格
        tableUtils.eachRange(zone.grid, {
            minY: zone.range.minY,
            maxY: maxRow,
            minX: zone.range.minX,
            maxX: maxCol
        }, function (cellData) {
            if (!cellData.fake) {
                cell = cellData.cell;
                pasteCell = gridPaste[(cellData.y - zone.range.minY) % pasteRowCount][(cellData.x - zone.range.minX) % pasteColCount];

                if (amend.isAmendEditing()) {
                    //修订模式 预处理
                    rangeUtils.setRange(cell, 0, cell.lastChild, domUtils.getEndOffset(cell.lastChild));
                    amendUtils.removeSelection(amendUser.getCurUser());
                    amendUtils.removeUserDel(cell, amendUser.getCurUser());

                    if (pasteCell.fake) {
                        return;
                    }

                    if (domUtils.isEmptyDom(cell)) {
                        cell.innerHTML = pasteCell.cell.innerHTML;
                    } else {
                        while (pasteCell.cell.firstChild) {
                            cell.appendChild(pasteCell.cell.firstChild);
                        }
                    }

                    amend.fixPaste(cell.firstChild, cell.lastChild, amendUser.getCurUser());

                } else {
                    cell.innerHTML = pasteCell.fake ? '' : pasteCell.cell.innerHTML;
                }
            }
        });
        //粘贴后，需要修订 range
        tableZone.setStart(zone.grid[zone.range.minY][zone.range.minX].cell)
            .setEnd(zone.grid[maxRow][maxCol].cell);
    }
}

var pasteUtils = {
    /**
     * 专门针对 IOS 的粘贴 操作 补丁
     * @param e
     * @returns {boolean}
     */
    pasteForIOS: function (e) {
        if (e) {
            utils.stopEvent(e);
        }
        var sel = ENV.doc.getSelection();

        //必须让 光标 消失然后再重新设置， 否则会导致 IOS 上一直显示 粘贴的 tooltip
        rangeUtils.backupCaret();
        sel.removeAllRanges();
        setTimeout(function () {
            rangeUtils.restoreCaret();
            if (e) {
                ENV.client.sendCmdToWiznote(CONST.CLIENT_EVENT.WizEditorPaste);
            }
        }, 0);
    },
    pasteFromClipBoard: function (e) {

        // CodeMirror 内 粘贴操作全部忽略
        if (codeUtils.getContainerFromChild(e.target)) {
            return false;
        }
        if (ENV.client.type.isIOS) {
            pasteUtils.pasteForIOS(e);
            return false;
        }

        var htmlSrc = e.clipboardData.getData('text/html') || '',
            txt = e.clipboardData.getData('text/plain');

        pasteUtils.paste(htmlSrc, txt);
        utils.stopEvent(e);
        return true;
    },
    pasteFromClient: function (html, txt) {
        pasteUtils.paste(html, txt);
    },
    paste: function (html, txt) {
        // console.log('-------------- htmlSrc -----------------');
        // console.log(html);

        html = clearClipHtml(html);
        html = utils.replaceSpecialChar(html);
        txt = utils.replaceSpecialChar(txt);

        // console.log('--------------   html   -----------------');
        // console.log(html);
        // console.log('--------------   txt   -----------------');
        // console.log(txt);

        var range = rangeUtils.getRange(),
            zone = tableZone.getZone();

        if ((!range && !zone.table && !zone.range) || (!html && !txt)) {
            return;
        }

        var cmContainer = codeUtils.getContainerFromChild(range.startContainer);
        if (cmContainer) {
            // CodeMirror 内 粘贴操作只插入 txt
            codeUtils.insertCodeSrc(cmContainer, txt);
            return;
        }

        historyUtils.saveSnap(false);

        if (!zone.table && !zone.range) {
            // 普通粘贴操作
            // if (!html) {
            //     // WizEditor 只干扰 html 粘贴， 纯文本使用浏览器默认功能
            //     return;
            // }
            pasteWithStandard(html, txt);
        } else {
            // 表格内粘贴
            // 表格内粘贴纯文本必须干预，否则会出现表格嵌套的情况
            pasteWithTable(html, txt);
        }
        domUtils.fixOrderList();

        var codeList = codeUtils.oldPatch.fixOldCode();
        var i, code;
        for (i = 0; i < codeList.length; i++) {
            code = codeList[i];
            codeUtils.fixCodeContainer(code);
        }
    }
};

module.exports = pasteUtils;

},{"../amend/amend":7,"../amend/amendUser":9,"../amend/amendUtils/amendExtend":11,"../codeUtils/codeUtils":16,"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../domUtils/xssUtils":31,"../rangeUtils/rangeExtend":49,"../tableUtils/tableCore":52,"../tableUtils/tableUtils":54,"../tableUtils/tableZone":55,"./commandExtend":33}],36:[function(require,module,exports){
/**
 * tab 键操作处理
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    domUtils = require('../domUtils/domExtend'),
    commandExtend = require('../editor/commandExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    tableZone = require('../tableUtils/tableZone'),
    todoUtils = require('../todoUtils/todoUtils'),
    historyUtils = require('../common/historyUtils');

var tabHtml = '&nbsp; &nbsp;&nbsp;';

function processTab (prev) {
    var range = rangeUtils.getRange(),
        zone = tableZone.getZone();
    if (!range || zone.range) {
        return false;
    }

    if (prev) {
        historyUtils.saveSnap(false);
        // mac 客户端 必须setTimeout 否则会导致崩溃
        if (ENV.client.type.isMac) {
            setTimeout(function () {
                commandExtend.execCommand("outdent");
            }, 10);
        } else {
            commandExtend.execCommand("outdent");
        }
        return true;
    }

    var dom = range.startContainer,
        startOffset = range.startOffset,
        startDetail = rangeUtils.getRangeDetail(dom, startOffset),
        isListDom = domUtils.isTag(dom, ['ul', 'ol', 'li']),
        isTodoStart = todoUtils.isCaretAfterCheckbox(),
        parent = domUtils.getParentByTagName(dom, ['ul', 'ol', 'li'], false),
        isListStart = false;

    var tmp = startDetail.container;
    if (parent && startDetail.offset === 0) {
        while (tmp !== ENV.doc.body) {
            if (!!tmp.previousSibling) {
                isListStart = false;
                break;
            } else {
                isListStart = true;
            }
            tmp = tmp.parentNode;
            if (tmp === parent) {
                break;
            }
        }
    }

    if (domUtils.isTag(dom, ['td', 'th'])) {
        return false;
    } else if (!range.collapsed || isTodoStart || isListStart || isListDom) {
        // 在 空列表、列表头部、todoList 头部 时， tab 键进行缩进操作
        historyUtils.saveSnap(false);
        if (ENV.client.type.isMac) {
            setTimeout(function () {
                commandExtend.execCommand("indent");
            }, 10);
        } else {
            commandExtend.execCommand("indent");
        }
        return true;
    } else if (dom.nodeType === 3 || domUtils.getParentByTagName(dom, ['a', 'b', 'body', 'div', 'font', 'html', 'i', 'p', 'span', 'strong', 'u'])) {
        historyUtils.saveSnap(false);
        commandExtend.execCommand("insertHTML", false, tabHtml);
        return true;
    }
    return false;
}

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
    },
    handler: {
        onKeyDown: function (e) {
            var keyCode = e.keyCode || e.which;
            if (keyCode !== 9) {
                return;
            }

            if (processTab(e.shiftKey)) {
                utils.stopEvent(e);
            }
        }
    }
};

var tabKey = {
    init: function (html) {
        tabHtml = html;
    },
    on: function () {
        _event.bind();
    },
    off: function () {
        _event.unbind();
    }
};

module.exports = tabKey;

},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../editor/commandExtend":33,"../rangeUtils/rangeExtend":49,"../tableUtils/tableZone":55,"../todoUtils/todoUtils":59}],37:[function(require,module,exports){
/**
 * 代码区域操作核心包 core
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    wizStyle = require('../common/wizStyle'),
    historyUtils = require('../common/historyUtils'),
    utils = require('../common/utils'),
    domUtils = require('../domUtils/domExtend'),
    commandExtend = require('../editor/commandExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    codeUtils = require('../codeUtils/codeUtils'),
    tableCore = require('../tableUtils/tableCore'),
    tableZone = require('../tableUtils/tableZone');

var FormatStyleList = ['background', 'background-color', 'color',
    'font-family', 'font-size', 'font-weight', 'font-style',
    'text-decoration', 'text-align'];
var FormatTitleList = ['h1', 'h2', 'h3', 'h4', 'h5', 'h6'];
var FormatSubSupList = ['sub', 'sup'];
var FormatFontList = ['b', 'i', 'u', 'strike'];
var FormatCommand = {
    'b': 'bold',
    'div': 'formatBlock',
    'h1': 'formatBlock',
    'h2': 'formatBlock',
    'h3': 'formatBlock',
    'h4': 'formatBlock',
    'h5': 'formatBlock',
    'h6': 'formatBlock',
    'i': 'italic',
    'strike': 'strikeThrough',
    'sub': 'subscript',
    'sup': 'superscript',
    'u': 'underline',
    'text-align-center': 'justifyCenter',
    'text-align-justify': 'justifyFull',
    'text-align-left': 'JustifyLeft',
    'text-align-right': 'JustifyRight'
};

var keepFormat = false;
var target;

function init () {
    target = {
        style: null,
        tagList: []
    }
}

function checkBlockTag (tagName) {
    return tagName.charAt(0) === 'h' || tagName === 'div';
}
function checkSubSupTag (tagName) {
    return /sub|sup/i.test(tagName);
}

function getStyle (start) {
    var styleList = FormatStyleList.concat(), styleResult = {};
    var tagList = FormatTitleList.concat(FormatSubSupList, FormatFontList);
    var tagMap = {}, tagName, tagObj;
    var hasTitle = false, hasSubSup = false, hasI = false, hasB = false;
    var startObj = start.nodeType === 1 ? start : start.parentNode;
    var obj = startObj, i;

    // 逐级查找 style 属性
    while (obj && obj !== ENV.doc.body) {
        analyseStyle(startObj, obj, styleList, styleResult);
        obj = obj.parentNode;
    }

    // 设置不存在的 style 为 null
    for (i = styleList.length - 1; i >= 0; i--) {
        styleResult[styleList[i]] = null;
    }
    target.style = styleResult;

    // 逐级查找 tag
    obj = start;
    while (obj) {
        obj = domUtils.getParentByTagName(obj, tagList, true);
        if (obj) {
            tagName = obj.tagName.toLowerCase();
            tagObj = {name: tagName, enabled: true};
            if (checkBlockTag(tagName)) {
                // 标题必须第一个处理，所以永远要写到首位
                target.tagList.splice(0, 0, tagObj);
                hasTitle = true;
            } else if (checkSubSupTag(tagName)) {
                target.tagList.push(tagObj);
                hasSubSup = true;
            } else {
                target.tagList.push(tagObj);
                // if (tagName.toLowerCase() === 'i') {
                if (/^i$/i.test(tagName)) {
                    hasI = true
                } else if (/^b$/i.test(tagName)) {
                    hasB = true
                }
            }
            tagMap[tagName] = 1;
            obj = obj.parentNode;
        }
    }
    if (!hasTitle) {
        target.tagList.splice(0, 0, {name: 'div', enabled: true});
    }
    if (!hasSubSup) {
        target.tagList.push({name: 'sub', enabled: false});
    }
    for (i = 0; i < tagList.length; i++) {
        tagName = tagList[i];
        if (checkBlockTag(tagName) || checkSubSupTag(tagName)) {
            // 所有不存在的标题 都不写入 tagList
            // sub & sup 不重复写入 tagList
            continue;
        }
        if (!tagMap[tagName]) {
            target.tagList.push({name: tagName, enabled: false});
        }
    }

    // 表格内 格式刷不处理 左右对齐
    var zone = tableZone.getZone();
    if (zone.range) {
        delete target.style['text-align'];
    } else {
        // 左右对齐需要使用 execCommand 操作
        if (target.style['text-align']) {
            tagName = 'text-align-' + target.style['text-align'];
        } else {
            tagName = 'text-align-left';
        }
        delete target.style['text-align'];
        target.tagList.push({name: tagName, enabled: true});
    }

    // console.log(target);
}

function analyseStyle (start, obj, styleList, result) {
    var name, style, i;
    var startComputeStyle = ENV.win.getComputedStyle(start);
    for (i = styleList.length - 1; i >= 0; i--) {
        name = styleList[i];
        style = obj.style[name];
        // 需要考虑 <font color="red"> 的情况
        if (!style && name === 'color' && domUtils.isTag(obj, 'font')) {
            style = obj.getAttribute(name);
        }
        if (style && domUtils.getFontSizeRem(style) === domUtils.getFontSizeRem(startComputeStyle[name])) {
            styleList.splice(i, 1);
            result[name] = style;
        }
    }
    // obj.style;
}


var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        ENV.event.add(CONST.EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);

    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);

    },
    handler: {
        onKeyDown: function (e) {
            var keyCode = e.keyCode || e.which;
            if (keyCode === 27) {
                formatPainterCore.off();
            }
        },
        onMouseUp: function () {
            if (!target.style) {
                return;
            }

            historyUtils.saveSnap(false);
            var range, start, end;

            var zone = tableZone.getZone();
            if (zone.range) {
                // 表格内 格式刷不处理 左右对齐
                delete target.tagList['text-align-left'];
                delete target.tagList['text-align-right'];
            }
            // 必须先执行 execCommand；因为修改样式会导致 range 改变
            var i, tag, commandName, commandState;
            for (i = 0; i < target.tagList.length; i++) {
                tag = target.tagList[i];
                commandName = FormatCommand[tag.name];
                if (checkBlockTag(tag.name)) {
                    // 处理 段落 参数与其他不同
                    commandExtend.execCommand(commandName, false, tag.name);
                    continue;
                }
                if (checkSubSupTag(tag.name)) {
                    // 处理 sub sup
                    if (tag.enabled) {
                        commandExtend.execCommand(commandName, false);
                    } else {
                        // 清理 sub & sup
                        commandExtend.clearSubSup();
                    }
                    continue;
                }
                commandState = commandExtend.queryCommandState(commandName);
                if (tag.enabled && !commandState) {
                    commandExtend.execCommand(commandName, false);
                } else if (!tag.enabled && commandState) {
                    commandExtend.execCommand(commandName, false);
                } else {
                    // 避免被选中的区域内有部分内容 commandState 为 true
                    // 浏览器表现不一致
                    // 例如 <b>...</b><span>...</span><b>...</b> 这范围内 得到的 commandState 有的为 true 有的为 false
                    // 保险起见，只要 目标与希望结果一致时，都需要执行两次
                    commandExtend.execCommand(commandName, false);
                    // Chrome bug：连续两次执行 execCommand 时，特殊情况会导致 range 得到的是错误结果
                    // 所以必须修正 range
                    range = rangeUtils.getRange();
                    if (range) {
                        start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
                        end = rangeUtils.getRangeDetail(range.endContainer, range.endOffset);
                    }
                    commandExtend.execCommand(commandName, false);
                    if (range) {
                        rangeUtils.setRange(start.container, start.offset,
                            end.container, end.offset);
                    }
                }
            }

            if (!tableCore.modifySelectionDom(target.style, null)) {
                // 处理普通文本样式
                rangeUtils.modifySelectionDom(target.style, null);
            }

            if (!keepFormat) {
                formatPainterCore.init();
            }
        }
    }
};


var formatPainterCore = {
    init: function () {
        formatPainterCore.off();
        // // todo 为便于测试，临时添加快捷键
        // ENV.event.add(CONST.EVENT.ON_KEY_UP, _event.handler.onKeyUp);
    },
    on: function (keep) {
        init();
        keepFormat = !!keep;

        if (formatPainterCore.status() === 0) {
            return 0;
        }

        var range = rangeUtils.getRange(),
            zone = tableZone.getZone();
        var start, startOffset = 0;
        if (range) {
            start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            startOffset = start.offset;
            start = start.container;
        } else {
            start = zone.start.cell;
        }
        start = domUtils.getFirstDeepChild(start);
        getStyle(start, startOffset);

        _event.bind();
        wizStyle.insertStyle({
            id: CONST.ID.FORMAT_PAINTER_STYLE,
            name: CONST.NAME.TMP_STYLE
        }, 'body {cursor:url("' + ENV.dependency.files.cursor.formatPainter + '"), auto;}');
        return 2;
    },
    off: function () {
        init();
        wizStyle.removeStyleById(CONST.ID.FORMAT_PAINTER_STYLE);
        _event.unbind();

        // todo 为便于测试，临时添加快捷键
        // ENV.event.remove(CONST.EVENT.ON_KEY_UP, _event.handler.onKeyUp);

        return formatPainterCore.status();
    },
    status: function () {
        if (ENV.readonly || ENV.client.type.isPhone || ENV.client.type.isPad) {
            // 阅读模式下所有客户端 以及 编辑模式下手机、Pad 禁止使用 格式刷
            return 0;
        }

        var range = rangeUtils.getRange(),
            zone = tableZone.getZone();
        if ((!range && !zone.range) || zone.active) {
            // 编辑区域无焦点，且不再 table 内，禁止使用 格式刷
            return 0;
        }

        if (range && codeUtils.getContainerFromChild(range.startContainer)) {
            // CodeMirror 内禁止使用 格式刷
            return 0;
        }

        // 关闭时 为 1； 使用时 为 2
        return (!!target && !!target.style) ? 2 : 1;
    }
};

module.exports = formatPainterCore;

},{"../codeUtils/codeUtils":16,"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../common/wizStyle":25,"../domUtils/domExtend":29,"../editor/commandExtend":33,"../rangeUtils/rangeExtend":49,"../tableUtils/tableCore":52,"../tableUtils/tableZone":55}],38:[function(require,module,exports){
/**
 * 超链接操作基本方法集合
 */
var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils'),
    codeUtils = require('./../codeUtils/codeUtils'),
    domUtils = require('./../domUtils/domExtend');
    // rangeUtils = require('./../rangeUtils/rangeExtend');

var defaultColor = 'yellow';
var activeColor = '#FF9632';
var hasHighlight = false;
var curTarget, curIndex,
    targetList = [];

var _class = {
    searching : 'cm-searching'
};

function highlightAll (key, focusFirst) {
    var sel;
    sel = ENV.win.getSelection();

    if (focusFirst) {
        curIndex = -1;
        curTarget = null;
        targetList = [];
    }
    hasHighlight = false;

    // 需要定位第一个匹配结果时，需要单独查找一次，获取第一次的 target
    if (focusFirst && ENV.win.find(key, false, false, true)) {
        curTarget = highlightTarget(key);
    }

    sel.collapse(ENV.doc.body, ENV.doc.body.childNodes.length);

    // 最多 高亮 100 次
    // 注意：样式-webkit-user-select: none; 会导致 find 方法永远都停留在某一个位置，从而出现死循环
    var count = 1;
    while (count++ < 101 && ENV.win.find(key, false, true)) {
        highlightTarget(key);
    }

    hasHighlight = !!ENV.doc.querySelector(CONST.TAG.TMP_HIGHLIGHT_TAG) ||
        !!ENV.doc.querySelector(CONST.TAG.CODE_MIRROR + ' .' + _class.searching);

    if (hasHighlight && !focusFirst) {
        sel.collapse(document.body, 0);
        sel.extend(document.body, 0);
    }
}

function highlightTarget (key) {
    var sel = ENV.win.getSelection(), range,
        item, tmp;
    range = sel.getRangeAt(0);

    // 如果需要高亮的目标已经高亮，直接跳过
    if (domUtils.getParentByTagName(range.startContainer, CONST.TAG.TMP_HIGHLIGHT_TAG)) {
        // 必须要让 range 折叠，否则有可能导致第一个目标出现阴影
        range.collapse(true);
        return null;
    }

    var codeContainer = codeUtils.getContainerFromChild(range.startContainer);
    if (codeContainer) {
        // 如果 container 查询过同关键字，忽略
        if (!codeContainer.state || codeContainer.state.query !== key) {
            codeUtils.highlight.search(codeContainer, key);
            if (!curTarget) {
                item = codeContainer.querySelector('.' + _class.searching);
            }
        }
    } else {
        item = document.createElement(CONST.TAG.TMP_HIGHLIGHT_TAG);
        domUtils.addClass(item, _class.searching);
        item.style.backgroundColor = defaultColor;

        tmp = range.extractContents();
        while (tmp.firstChild) {
            item.appendChild(tmp.firstChild);
        }

        // 如果选中的内容为 input 内 item 将为空
        if (!domUtils.isEmptyDom(item)) {
            range.insertNode(item);
        }
    }
    return item;
}

function initTargetList () {
    targetList = ENV.doc.querySelectorAll('.' + _class.searching);
    if (targetList.length === 0) {
        return;
    }
    var target, i;
    for (i = targetList.length - 1; i >= 0; i--) {
        target = targetList[i];
        if (target === curTarget) {
            curIndex = i;
            return;
        }
    }
    // 如果没有找到 curTarget 则设置为第一个 target
    curIndex = 0;
    curTarget = targetList[0];
}

function setTargetFocus (index) {
    var lastIndex = curIndex;

    if (targetList.length === 0) {
        return;
    }

    // 实现循环搜索
    if (index >= targetList.length) {
        index = 0;
    } else if (index < 0) {
        index = targetList.length - 1;
    }

    // 如果需要设置的 index 与 上一次的相同，则不操作
    if (index === lastIndex) {
        curTarget.style.backgroundColor = activeColor;
        return;
    }

    targetList[lastIndex].style.backgroundColor = defaultColor;
    curIndex = index;
    curTarget = targetList[curIndex];
    curTarget.style.backgroundColor = activeColor;

    if (curTarget.scrollIntoViewIfNeeded) {
        curTarget.scrollIntoViewIfNeeded();
    }
}

// function highlightSingle(key, aBackwards, rangeStart) {
//     var sel, range, item,
//         tmp;
//     sel = ENV.win.getSelection();
//
//     if (rangeStart === -1) {
//         sel.collapse(ENV.doc.body, 0);
//         sel.extend(document.body, 0);
//     } else if (rangeStart === 1) {
//         sel.collapse(ENV.doc.body, ENV.doc.body.childNodes.length);
//         sel.extend(document.body, ENV.doc.body.childNodes.length);
//     }
//
//     if (ENV.win.find(key, false, aBackwards, true)) {
//         item = document.createElement(CONST.TAG.TMP_HIGHLIGHT_TAG);
//         item.style.backgroundColor = 'yellow';
//         range = sel.getRangeAt(0);
//         tmp = range.extractContents();
//         while (tmp.firstChild) {
//             item.appendChild(tmp.firstChild);
//         }
//         // 如果选中的内容为 input 内  item 将为空
//         if (!domUtils.isEmptyDom(item)) {
//             range.insertNode(item);
//             if (!aBackwards) {
//                 rangeUtils.setRange(item, item.childNodes.length);
//             }
//             item.scrollIntoViewIfNeeded();
//         } else if (aBackwards) {
//             sel.collapseToStart();
//         } else {
//             sel.collapseToEnd();
//         }
//         return true;
//     }
//     return false;
// }

/**
 * 根据输入内容 自动匹配并生成 超链接 <a>
 */
var highlightUtils = {
    on: function (keyList, focusFirst) {
        highlightUtils.off();
        if (!keyList) {
            return false;
        }
        if (!utils.isArray(keyList)) {
            keyList = [keyList];
        }
        // 关键字数量 大于 1 的时候，不实现 第一个搜索结果高亮
        if (keyList.length > 1) {
            focusFirst = false;
        }
        var i, j;
        for (i = 0, j = keyList.length; i < j; i++) {
            highlightAll(keyList[i], focusFirst);
        }

        if (focusFirst) {
            initTargetList();
            setTargetFocus(curIndex);
        }
        return hasHighlight;
    },
    off: function () {
        domUtils.peelTag(CONST.TAG.TMP_HIGHLIGHT_TAG);
        codeUtils.highlight.clearAll();
    },
    next: function () {
        setTargetFocus(curIndex + 1);
    },
    previous: function () {
        setTargetFocus(curIndex - 1);
    }
};

module.exports = highlightUtils;
},{"./../codeUtils/codeUtils":16,"./../common/const":18,"./../common/env":20,"./../common/utils":24,"./../domUtils/domExtend":29}],39:[function(require,module,exports){
/**
 * 点击 Img 操作
 */

var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils'),
    domUtils = require('./../domUtils/domExtend'),
    imgUtils = require('./imgUtils');

function imgCheck(img) {
    var imgClassExp = new RegExp('(' + CONST.CLASS.IMG_NOT_DRAG + ')|(' + CONST.CLASS.IMG_ATTACHMENT + ')', 'i');
    var result = !!(img && domUtils.isTag(img, 'img') && !imgClassExp.test(img.className) && img.src);
    if (!result) {
        return result;
    }

    var p;
    if (ENV.readonly) {
        // 阅读模式
        // 对于 超链接内的 img 不阻止点击事件，因为响应 超链接 更重要
        p = domUtils.getParentByFilter(img, function (node) {
            return (node && domUtils.isTag(node, 'a') && (/^(http|https|wiz|wiznote|wiznotecmd):/.test(node.getAttribute('href'))));
        }, true);
        return !p;

    } else {
        // 编辑模式
        // 对于 添加附件 生成的 img 不允许编辑
        p = domUtils.getParentByFilter(img, function (node) {
            return (node && domUtils.isTag(node, 'a') && (/^(wiz|wiznote|wiznotecmd):open_attachment/.test(node.getAttribute('href'))));
        }, true);
        return !p;
    }
}

function clickImgForRead(e) {
    var target = e.target;
    if (!imgCheck(target)) {
        return;
    }

    ENV.client.sendCmdToWiznote(CONST.CLIENT_EVENT.WizReaderClickImg, {
        src: target.src,
        imgList: ENV.client.type.isIOS ? null : imgUtils.getAll(true)
    });
    utils.stopEvent(e);
    return false;
}

function clickImgForEdit(e) {
    var target = e.target;
    if (!imgCheck(target)) {
        return;
    }

    // 删除之前遗漏的 img 状态
    var imgList = ENV.doc.querySelectorAll('img[' + CONST.ATTR.IMG_EDITING + ']'),
        i, attr = {};
    attr[CONST.ATTR.IMG_EDITING] = '';
    for (i = imgList.length - 1; i >= 0; i--) {
        domUtils.attr(imgList[i], attr);
    }

    attr[CONST.ATTR.IMG_EDITING] = 1;
    domUtils.attr(target, attr);

    ENV.client.sendCmdToWiznote(CONST.CLIENT_EVENT.WizEditorClickImg, {
        src: target.src
    });
}

function init() {
}

var _event = {
    bind: function () {
        _event.unbind();
        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
          ENV.event.add(CONST.EVENT.ON_CLICK, _event.handler.onClick);
        } else if (ENV.readonly &&
          (ENV.client.type.isWin || ENV.client.type.isMac || ENV.client.type.isLinux)) {
          ENV.event.add(CONST.EVENT.ON_DBLCLICK, _event.handler.onClick);
        }

    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_CLICK, _event.handler.onClick);
    },
    handler: {
        onClick: function (e) {
            if (ENV.readonly) {
                clickImgForRead(e);
            } else if (ENV.client.type.isAndroid) {
                clickImgForEdit(e);
            }
        }
    }
};

var imgClick = {
    init: init,
    bind: _event.bind,
    unbind: _event.unbind
};

module.exports = imgClick;


},{"./../common/const":18,"./../common/env":20,"./../common/utils":24,"./../domUtils/domExtend":29,"./imgUtils":42}],40:[function(require,module,exports){
/**
 * img 组件注册
 */

var ENV = require('./../common/env'),
    imgResize = require('./imgResize'),
    imgClick = require('./imgClick');

var imgCore = {
    on: function () {
        if (!ENV.client.type.isPhone && !ENV.client.type.isPad) {
            imgResize.init();
            imgResize.bind();
        }

        imgClick.init();
        imgClick.bind();
    },
    off: function () {
        if (!ENV.client.type.isPhone && !ENV.client.type.isPad) {
            imgResize.unbind();
        }
        imgClick.unbind();
    },
    setImgFullPath: function() {
        // 为了保证 客户端不同笔记之间复制图片能够正常获取 路径
        var imgList = ENV.doc.querySelectorAll('img');
        var i, img, src;
        var indexFilesPathReg = new RegExp('^(\\.\\/)?' + ENV.options.indexFilesPath.escapeRegex() + '\\/', 'i');
        // console.log(indexFilesPathReg);
        for (i=imgList.length -1; i>=0; i--) {
            img = imgList[i];
            src = img.getAttribute('src');
            // console.log(src);
            if (indexFilesPathReg.test(src)) {
                img.setAttribute('src',
                    src.replace(indexFilesPathReg, ENV.options.indexFilesFullPath));
            }
        }
    }
};

module.exports = imgCore;

},{"./../common/env":20,"./imgClick":39,"./imgResize":41}],41:[function(require,module,exports){
/**
 * img Resize 工具
 */

var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils'),
    domUtils = require('./../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend');

var handleSuffix = ['lt', 'tm', 'rt', 'rm', 'rb', 'bm', 'lb', 'lm'];

var resizingHanlde = '';
var WIZ_STYLE = 'wiz_style';

var startOffsetX;
var startOffsetY;
var lastMousex;
var lastMousey;
var oppCornerX;
var oppCornerY;

// patch Window Client WebView bug
// 修正 Chrome 内核 bug —— 图片未选中时，无法拖拽
var dragImgStart = false;
var dragImg = null;


var curImg;
var cursorOri;
var cursor;

function init () {
    cursorOri = ENV.doc.body.style.cursor || '';

    // TODO 临时为 pc端处理，pc端整合后， 直接删除
    ENV.win.WizImgResizeOnGetHTML = function () {
    };
}

function initImageDragResize (img) {
    if (!img || !img.tagName || img.tagName.toLowerCase() != 'img')
        return;
    if (!canDragResize(img))
        return;
    //
    var container = createHandles();
    if (!container) {
        return;
    }
    resetHandlesSize(img);
    initImage(img);

    _event.bindContainer(container);
}

function clearHandles () {
    removeImgAttributes();
    removeHandles();
    curImg = null;
    ENV.doc.body.style.cursor = cursorOri;
}

function createHandles () {
    var container = getHandleContainer();
    if (container) {
        return container;
    }
    container = ENV.doc.createElement(CONST.TAG.TMP_TAG);
    domUtils.addClass(container, CONST.CLASS.IMG_RESIZE_CONTAINER);
    container.setAttribute('contenteditable', 'false');
    container.setAttribute(WIZ_STYLE, 'unsave');

    for (var i = 0; i < handleSuffix.length; i++) {
        var handle = ENV.doc.createElement('div');
        domUtils.addClass(handle, CONST.CLASS.IMG_RESIZE_HANDLE);
        domUtils.addClass(handle, handleSuffix[i]);
        domUtils.attr(handle, {
            'data-type': handleSuffix[i]
        });
        container.appendChild(handle);
    }
    ENV.doc.body.appendChild(container);
    return container;
}

function getHandleContainer () {
    var container = ENV.doc.body.querySelector('.' + CONST.CLASS.IMG_RESIZE_CONTAINER);
    if (!container || container.length < 1) {
        return null;
    }
    return container;
}

function setHandleSize (imgOptions, handle) {
    if (!imgOptions || !handle)
        return;
    var offset = imgOptions.offset;
    var x = offset.left, y = offset.top,
        width = imgOptions.width, height = imgOptions.height;

    var handleName = handle.getAttribute('data-type');
    var left = 0, top = 0;
    switch (handleName) {
        case 'lt':
            left = x - 7;
            top = y - 7;
            break;
        case 'tm':
            left = x + (width - 7) / 2;
            top = y - 7;
            break;
        case 'rt':
            left = x + width;
            top = y - 7;
            break;
        case 'rm':
            left = x + width;
            top = y + (height - 7) / 2;
            break;
        case 'rb':
            left = x + width;
            top = y + height;
            break;
        case 'bm':
            left = x + (width - 7) / 2;
            top = y + height;
            break;
        case 'lb':
            left = x - 7;
            top = y + height;
            break;
        case 'lm':
            left = x - 7;
            top = y + (height - 7) / 2;
            break;
    }
    domUtils.css(handle, {
        left: left + 'px',
        top: top + 'px'
    });
}

function resetHandlesSize (img) {
    if (!img) {
        return;
    }
    var container = getHandleContainer();
    if (!container) {
        return;
    }
    var handles = container.querySelectorAll('.' + CONST.CLASS.IMG_RESIZE_HANDLE);

    var imgOptions = {
        offset: domUtils.getOffset(img),
        width: img.width,
        height: img.height
    };
    for (var i = 0; i < handles.length; i++) {
        var handle = handles[i];
        setHandleSize(imgOptions, handle);
        handle.style.visibility = 'inherit';
    }
}

function removeImgAttributes () {
    var imgList = ENV.doc.querySelectorAll('.' + CONST.CLASS.IMG_RESIZE_ACTIVE);
    if (!imgList || imgList.length === 0) {
        return;
    }
    var i;
    for (i = imgList.length - 1; i >= 0; i--) {
        domUtils.removeClass(imgList[i], CONST.CLASS.IMG_RESIZE_ACTIVE);
    }
}

function removeHandles () {
    var container = getHandleContainer();
    if (!container) {
        return;
    }
    _event.unbindContainer(container);
    domUtils.remove(container);
}

function initImage (img) {
    if (!img) {
        return;
    }
    removeImgAttributes();
    domUtils.addClass(img, CONST.CLASS.IMG_RESIZE_ACTIVE);
    img.attributes[CONST.ATTR.IMG_RATE] = img.width / img.height;
    curImg = img;
    var imgIndex = domUtils.getIndex(img);
    rangeUtils.setRange(img.parentNode, imgIndex, img.parentNode, imgIndex + 1);
}

function canDragResize (img) {
    if (!img)
        return false;
    //
    var className = img.getAttribute('class');
    if (className && -1 !== className.indexOf(CONST.CLASS.IMG_NOT_DRAG))
        return false;
    //
    return true;
}

function showHandles (show) {
    var container = getHandleContainer();
    if (!container) {
        return;
    }
    container.style.display = show ? 'block' : 'none';

    if (!show) {
        clearHandles();
    }
}
function scaleImgSize (rate, widthDraged, heightDraged, img) {
    if (!img)
        return;
    //
    var widthSized = heightDraged * rate;
    var heightSized = widthDraged / rate;
    //
    if (widthSized < widthDraged)
        widthSized = widthDraged;
    else
        heightSized = heightDraged;
    //
    img.width = widthSized;
    img.height = heightSized;
}

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.BEFORE_GET_DOCHTML, _event.handler.beforeGetDocHtml);
        ENV.event.add(CONST.EVENT.ON_CUT, _event.handler.onCut);
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        ENV.event.add(CONST.EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
        ENV.event.add(CONST.EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
    },
    bindContainer: function (container) {
        _event.unbindContainer(container);
        container.addEventListener('mousedown', _event.handler.onContainerMouseDown);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.BEFORE_GET_DOCHTML, _event.handler.beforeGetDocHtml);
        ENV.event.remove(CONST.EVENT.ON_CUT, _event.handler.onCut);
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_MOVE, _event.handler.onMouseMove);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
    },
    unbindContainer: function (container) {
        container.removeEventListener('mousedown', _event.handler.onContainerMouseDown);
    },
    handler: {
        beforeGetDocHtml: function () {
            clearHandles();
        },
        onCut: function (e) {
            showHandles(false);
        },
        onKeyDown: function (e) {
            var keyCode = e.keyCode || e.which;
            var start, startOffset;
            if (curImg && (keyCode === 8 || keyCode === 46)) {
                start = curImg.parentNode;
                if (start) {
                    startOffset = domUtils.getIndex(curImg);
                    rangeUtils.setRange(start, startOffset, start, startOffset + 1);
                }
            }
            showHandles(false);
        },
        onContainerMouseDown: function (e) {
            var elm = e.target || e.srcElement;
            resizingHanlde = elm.getAttribute('data-type');
            var img = ENV.doc.querySelector('.' + CONST.CLASS.IMG_RESIZE_ACTIVE);
            var mousex, mousey, offset;
            if (!img) {
                return;
            }

            mousex = e.pageX;
            mousey = e.pageY;
            offset = domUtils.getOffset(img);
            //
            switch (resizingHanlde) {
                case 'lt':
                    startOffsetX = offset.left - mousex;
                    startOffsetY = offset.top - mousey;
                    //
                    oppCornerX = offset.left + img.width;
                    oppCornerY = offset.top + img.height;
                    //
                    cursor = 'nw-resize';
                    break;
                case 'tm':
                    startOffsetX = undefined;
                    startOffsetY = offset.top - mousey;
                    //
                    cursor = 'n-resize';
                    break;
                case 'rt':
                    startOffsetX = mousex - img.width - offset.left;
                    startOffsetY = offset.top - mousey;
                    //
                    oppCornerX = offset.left;
                    oppCornerY = offset.top + img.height;
                    //
                    cursor = 'ne-resize';
                    break;
                case 'rm':
                    startOffsetX = mousex - img.width - offset.left;
                    startOffsetY = undefined;
                    //
                    cursor = 'e-resize';
                    break;
                case 'rb':
                    startOffsetX = mousex - img.width - offset.left;
                    startOffsetY = mousey - img.height - offset.top;
                    //
                    cursor = 'se-resize';
                    break;
                case 'bm':
                    startOffsetX = undefined;
                    startOffsetY = mousey - img.height - offset.top;
                    //
                    oppCornerX = offset.left / 2;
                    oppCornerY = offset.top;
                    //
                    cursor = 's-resize';
                    break;
                case 'lb':
                    startOffsetX = offset.left - mousex;
                    startOffsetY = mousey - img.height - offset.top;
                    //
                    oppCornerX = offset.left + img.width;
                    oppCornerY = offset.top;
                    //
                    cursor = 'sw-resize';
                    break;
                case 'lm':
                    startOffsetX = offset.left - mousex;
                    startOffsetY = undefined;
                    //
                    cursor = 'w-resize';
                    break;
            }
            utils.stopEvent(e);
        },
        onDrop: function (e) {
            ENV.doc.removeEventListener('drop', _event.handler.onDrop);
            if (curImg === dragImg) {
                clearHandles();
            }
            dragImgStart = false;
            dragImg = null;
        },
        onMouseDown: function (e) {
            var elm = e.target || e.srcElement;
            if (domUtils.isTag(elm, 'img')) {
                dragImgStart = true;
                dragImg = elm;
                ENV.doc.addEventListener('drop', _event.handler.onDrop);
            } else {
                dragImgStart = false;
                dragImg = null;
            }
            if (e.target === curImg) {
                return;
            }
            showHandles(false);
            removeImgAttributes();
        },
        onMouseMove: function (e) {
            var offset, mousex, mousey;
            var index, img;
            var elm = e.target || e.srcElement;

            if (elm && elm === dragImg) {
                index = domUtils.getIndex(e.target);
                img = elm;
                rangeUtils.setRange(img.parentNode, index, img.parentNode, index + 1);
            }

            img = ENV.doc.querySelector('.' + CONST.CLASS.IMG_RESIZE_ACTIVE);
            if (!img) {
                return;
            }
            offset = domUtils.getOffset(img);
            //
            if (resizingHanlde) {
                //
                mousex = e.pageX;
                mousey = e.pageY;
                //
                ENV.doc.body.style.cursor = cursor;
                // console.log('mousex: ' + mousex + ', mousey: ' + mousey);
                // console.log('lastMousex: ' + lastMousex + ', lastMousey: ' + lastMousey);
                var rate;
                var widthDraged;
                var heightDraged;
                var widthSized;
                var heightSized;
                //
                if (!lastMousex || !lastMousey) {
                    lastMousex = mousex;
                    lastMousey = mousey;
                }
                //
                switch (resizingHanlde) {
                    case 'tm':
                        img.width = img.width;
                        if (mousey < offset.top) {
                            img.height += lastMousey - mousey;
                        } else {
                            heightSized = img.height - (mousey - lastMousey) - startOffsetY;
                            img.height = heightSized < 0 ? 0 : heightSized;
                        }
                        break;
                    case 'rm':
                        widthSized = mousex - offset.left - startOffsetX;
                        img.width = widthSized < 0 ? 0 : widthSized;
                        img.height = img.height;
                        img.attributes[CONST.ATTR.IMG_RATE] = img.width / img.height;
                        break;
                    case 'bm':
                        img.width = img.width;
                        heightSized = mousey - oppCornerY - startOffsetY;
                        img.height = heightSized < 0 ? 0 : heightSized;
                        img.attributes[CONST.ATTR.IMG_RATE] = img.width / img.height;
                        break;
                    case 'lm':
                        img.height = img.height;
                        if (mousex < offset.left) {
                            img.width += lastMousex - mousex;
                        } else {
                            widthSized = img.width - (mousex - lastMousex) - startOffsetX;
                            img.width = widthSized < 0 ? 0 : widthSized;
                        }
                        break;
                    case 'lt':
                        rate = Number(img.attributes[CONST.ATTR.IMG_RATE]);
                        //
                        widthDraged = oppCornerX - mousex;
                        heightDraged = oppCornerY - mousey;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        break;
                    case 'rt':
                        rate = Number(img.attributes[CONST.ATTR.IMG_RATE]);
                        //
                        widthDraged = mousex - oppCornerX;
                        heightDraged = oppCornerY - mousey;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        break;
                    case 'lb':
                        rate = Number(img.attributes[CONST.ATTR.IMG_RATE]);
                        //
                        widthDraged = oppCornerX - mousex;
                        heightDraged = mousey - oppCornerY;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        break;
                    case 'rb':
                        rate = Number(img.attributes[CONST.ATTR.IMG_RATE]);
                        // console.log('mousex: ' + mousex + 'mousey: ' + mousey);
                        widthDraged = mousex - offset.left;
                        heightDraged = mousey - offset.top;
                        //
                        widthDraged -= startOffsetX;
                        heightDraged -= startOffsetY;
                        //
                        widthDraged = widthDraged < 0 ? 0 : widthDraged;
                        heightDraged = heightDraged < 0 ? 0 : heightDraged;
                        //
                        scaleImgSize(rate, widthDraged, heightDraged, img);
                        //
                        // console.log('rate: ' + rate + ', ' + 'widthDraged: ' + widthDraged + ', ' + 'heightDraged: ' + heightDraged + ', ' + 'widthSized: ' +
                        // 	widthSized + ', ' + 'heightSized: ' + heightSized);
                        break;
                }
                //
                if (img.style.cssText) {
                    var cssText = img.style.cssText;
                    cssText = cssText.replace(/width:\s*\d+.?\d+px;?/ig, 'width: ' + img.width + 'px')
                        .replace(/height:\s*\d+.?\d+px;?/ig, 'height: ' + img.height + 'px');
                    //
                    img.style.cssText = cssText;
                }
                //
                lastMousex = mousex;
                lastMousey = mousey;

                resetHandlesSize(img);
                ENV.event.call(CONST.EVENT.UPDATE_RENDER, null);
            }
        },
        onMouseUp: function (e) {
            ENV.doc.removeEventListener('drop', _event.handler.onDrop);
            dragImgStart = false;
            dragImg = null;

            var elm = e.target || e.srcElement;

            if (domUtils.isTag(elm, 'img')) {
                initImageDragResize(elm);
            }
            //
            resizingHanlde = '';
            //
            lastMousex = undefined;
            lastMousey = undefined;
            //
            oppCornerX = undefined;
            oppCornerY = undefined;
            //
            startOffsetX = undefined;
            startOffsetY = undefined;
            //
            ENV.doc.body.style.cursor = cursorOri;
        }
    }
};

var imgResize = {
    init: init,
    bind: _event.bind,
    unbind: _event.unbind
};

module.exports = imgResize;


},{"../rangeUtils/rangeExtend":49,"./../common/const":18,"./../common/env":20,"./../common/utils":24,"./../domUtils/domExtend":29}],42:[function(require,module,exports){
/**
 * img 操作基本方法集合
 */

var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    domUtils = require('./../domUtils/domBase');

var imgUtils = {
    getAll: function (onlyLocal) {
        var images = ENV.doc.images,
            img, imageSrcs = [],
            tmp = {}, src;
        for (img in images) {
            if (images.hasOwnProperty(img)) {
                //有特殊字符的文件名， 得到src 时是被转义后的名字，所以必须 decode 处理
                src = decodeURIComponent(images[img].src);
                if (imgFilter(images[img], onlyLocal) && !tmp[src]) {
                    imageSrcs.push(src);
                    tmp[src] = true;
                }
            }
        }
        return imageSrcs;
    },
    getImageSize: function (imgSrc) {
        var newImg = new Image();
        newImg.src = imgSrc;
        var height = newImg.height;
        var width = newImg.width;
        return {width: width, height: height};
    },
    getImageData: function (img) {
        var size = imgUtils.getImageSize(img.src);
        // Create an empty canvas element
        var canvas = ENV.doc.createElement("canvas");
        canvas.width = size.width;
        canvas.height = size.height;

        // Copy the image contents to the canvas
        var ctx = canvas.getContext("2d");
        ctx.drawImage(img, 0, 0);

        // Get the data-URL formatted image
        // Firefox supports PNG and JPEG. You could check img.src to
        // guess the original format, but be aware the using "image/jpg"
        // will re-encode the image.
        var dataURL = canvas.toDataURL("image/png");

        return dataURL.replace(/^data:image\/(png|jpg);base64,/, "");
    },
    makeAttachmentDom: function (guid, imgPath) {
        var result = [], main, a, img;
        main = ENV.doc.createElement("div");
        main.style.margin = '10px auto';
        a = ENV.doc.createElement("a");
        a.href = 'wiz://open_attachment?guid=' + guid;
        img = ENV.doc.createElement("img");
        img.src = imgPath;
        img.style.verticalAlign = 'bottom';
        img.style.maxWidth = '280px';
        domUtils.addClass(img, CONST.CLASS.IMG_ATTACHMENT);
        a.appendChild(img);
        main.appendChild(a);
        result.push(main);
        main = ENV.doc.createElement("div");
        main.appendChild(ENV.doc.createElement("br"));
        result.push(main);

        return result;
    },
    makeDomByPath: function (imgPath) {
        var result = [], paths = [],
            main, img, i, j;
        if (imgPath.indexOf('*')) {
            paths = imgPath.split("*");
        } else {
            paths.push(imgPath);
        }

        for (i = 0, j = paths.length; i < j; i++) {
            main = ENV.doc.createElement("div");
            result.push(main);

            img = ENV.doc.createElement("img");
            img.src = paths[i];
            img.style.verticalAlign = 'bottom';
            img.style.maxWidth = '100%';
            main.insertBefore(img, null);

            main = ENV.doc.createElement("div");
            main.insertBefore(ENV.doc.createElement("br"), null);
            result.push(main);
        }
        // main = ENV.doc.createElement("div");
        // main.insertBefore(ENV.doc.createElement("br"), null);
        // result.push(main);
        return result;
    },
    remove: function (selector) {
        var imgList = ENV.doc.querySelectorAll(selector),
            img, i;
        for (i = imgList.length - 1; i >= 0; i--) {
            img = imgList[i];
            img.parentElement.removeChild(img);
        }
    },
    replaceSrc: function (selector, targetSrc) {
        var imgList = ENV.doc.querySelectorAll(selector),
            img, i, attr = {};
        attr[CONST.ATTR.IMG_EDITING] = '';
        for (i = imgList.length - 1; i >= 0; i--) {
            img = imgList[i];
            img.src = targetSrc;
            domUtils.attr(img, attr);
        }
    }
};

function imgFilter(img, onlyLocal) {
    if (!img || (img.className && img.className.indexOf('wiz-todo') > -1)) {
        //checklist 的图片不进行获取
        return false;
    }
    var path = img.src;
    if (!path) {
        return false;
    }
    var rLocal = /^(http|https|ftp):/,
        rNoBase64 = /^(data):/,
        result;

    result = !rNoBase64.test(path);
    if (!result || !onlyLocal) {
        return result;
    }
    return !rLocal.test(path);
}

module.exports = imgUtils;

},{"./../common/const":18,"./../common/env":20,"./../domUtils/domBase":28}],43:[function(require,module,exports){
/**
 * 超链接操作基本方法集合
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    historyUtils = require('../common/historyUtils'),
    domUtils = require('../domUtils/domExtend'),
    commandExtend = require('../editor/commandExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend');

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_KEY_DOWN, _event.handler.onKeyDown);
    },
    handler: {
        onKeyDown: function (e) {
            var keyCode = e.keyCode || e.which;
            var start, next;
            var sel, range, offset, charCode;
            if (keyCode === 32 || keyCode === 13) {
                historyUtils.saveSnap(false);

                sel = ENV.doc.getSelection();
                range = sel.getRangeAt(0).cloneRange();

                start = range.startContainer;
                while (start.nodeType === 1 && range.startOffset > 0) {
                    start = range.startContainer.childNodes[range.startOffset - 1];
                    if (!start) {
                        break;
                    }
                    range.setStart(start, domUtils.getEndOffset(start));
                    range.collapse(true);
                    start = range.startContainer;
                }
                do {
                    if (range.startOffset === 0) {
                        start = range.startContainer.previousSibling;

                        while (start && start.nodeType === 1) {
                            start = start.lastChild;
                        }
                        if (!start || domUtils.isFillChar(start, false)) {
                            break;
                        }
                        offset = start.nodeValue.length;
                    } else {
                        start = range.startContainer;
                        offset = range.startOffset;
                    }
                    range.setStart(start, offset - 1);
                    charCode = range.toString().charCodeAt(0);
                } while (charCode !== 160 && charCode !== 32);

                if (range.toString().replace(CONST.FILL_CHAR_REG, '').match(/(?:https?:\/\/|ssh:\/\/|ftp:\/\/|file:\/|www\.)/i)) {
                    while (range.toString().length) {
                        if (/^(?:https?:\/\/|ssh:\/\/|ftp:\/\/|file:\/|www\.)/i.test(range.toString())) {
                            break;
                        }
                        try {
                            range.setStart(range.startContainer, range.startOffset + 1);
                        } catch (e) {
                            //trace:2121
                            start = range.startContainer;
                            while (!(next = start.nextSibling)) {
                                if (domUtils.isBody(start)) {
                                    return;
                                }
                                start = start.parentNode;
                            }
                            range.setStart(next, 0);

                        }

                    }
                    //if is <a>, then return;
                    if (domUtils.getParentByTagName(range.startContainer, 'a', true, null)) {
                        return;
                    }

                    var a = ENV.doc.createElement('a'), text = ENV.doc.createTextNode(' '), href;

                    var rangeText = range.extractContents();
                    a.href = rangeText.textContent;
                    a.appendChild(ENV.doc.createTextNode(rangeText.textContent));
                    href = a.getAttribute("href").replace(CONST.FILL_CHAR_REG, '');
                    href = /^(?:https?:\/\/)/ig.test(href) ? href : "http://" + href;
//                    a.setAttribute('_src', href);
                    a.href = href;

                    range.insertNode(a);
                    a.parentNode.insertBefore(text, a.nextSibling);
                    range.setStart(text, 0);
                    range.collapse(true);
                    sel.removeAllRanges();
                    sel.addRange(range);
                }
            }
        }
    }
};
/**
 * 根据输入内容 自动匹配并生成 超链接 <a>
 */
var linkUtils = {
    on: function () {
        _event.bind();
    },
    off: function () {
        _event.unbind();
    },
    getCurrentLink: function () {
        var range = rangeUtils.getRange();
        if (!range) {
            return '';
        }
        var currentNode = domUtils.getParentByTagName(range.startContainer, 'a', true);
        if (!currentNode) {
            return '';
        }
        return currentNode.href;
    },
    /**
     * 移除选中的 <a> 标签的超链接
     */
    removeSelectedLink: function () {
        fixARange();
        historyUtils.saveSnap(false);
        commandExtend.execCommand("unlink", false, false);
    },
    setCurrentLink: function (url) {
        if (!url) {
            return;
        }
        fixARange();
        historyUtils.saveSnap(false);
        commandExtend.execCommand("createLink", false, url);
    }
};

/**
 * 修正 选择范围，如果 rang 的 start 或 end 在 A 中间，则选中 A 的全部
 */
function fixARange() {
    var range = rangeUtils.getRange(),
        start, startOffset, end, endOffset;
    if (!range) {
        return;
    }
    start = getA(range.startContainer, range.startOffset);
    if (start) {
        startOffset = 0;
    } else {
        start = range.startContainer;
        startOffset = range.startOffset;
    }
    if (range.collapsed) {
        end = start;
        endOffset = domUtils.getEndOffset(start);
    } else {
        end = getA(range.endContainer, range.endOffset);
        if (end) {
            endOffset = domUtils.getEndOffset(end);
        } else {
            end = range.endContainer;
            endOffset = range.endOffset;
        }
    }
    rangeUtils.setRange(start, startOffset, end, endOffset);
}

/**
 * 根据 Range 的 Container 和 Offset 获取 对应的 A
 * @param target
 * @param offset
 * @returns {*}
 */
function getA(target, offset) {
    if (target.nodeType === 1 && target.childNodes.length > offset) {
        target = target.childNodes[offset];
    }
    return domUtils.getParentByTagName(target, 'a', true);
}

module.exports = linkUtils;

},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../domUtils/domExtend":29,"../editor/commandExtend":33,"../rangeUtils/rangeExtend":49}],44:[function(require,module,exports){
"use strict";
var Markdown;

if (typeof exports === "object" && typeof require === "function") // we're in a CommonJS (e.g. Node.js) module
    Markdown = exports;
else
    Markdown = {};

// The following text is included for historical reasons, but should
// be taken with a pinch of salt; it's not all true anymore.

//
// Wherever possible, Showdown is a straight, line-by-line port
// of the Perl version of Markdown.
//
// This is not a normal parser design; it's basically just a
// series of string substitutions.  It's hard to read and
// maintain this way,  but keeping Showdown close to the original
// design makes it easier to port new features.
//
// More importantly, Showdown behaves like markdown.pl in most
// edge cases.  So web applications can do client-side preview
// in Javascript, and then build identical HTML on the server.
//
// This port needs the new RegExp functionality of ECMA 262,
// 3rd Edition (i.e. Javascript 1.5).  Most modern web browsers
// should do fine.  Even with the new regular expression features,
// We do a lot of work to emulate Perl's regex functionality.
// The tricky changes in this file mostly have the "attacklab:"
// label.  Major or self-explanatory changes don't.
//
// Smart diff tools like Araxis Merge will be able to match up
// this file with markdown.pl in a useful way.  A little tweaking
// helps: in a copy of markdown.pl, replace "#" with "//" and
// replace "$text" with "text".  Be sure to ignore whitespace
// and line endings.
//


//
// Usage:
//
//   var text = "Markdown *rocks*.";
//
//   var converter = new Markdown.Converter();
//   var html = converter.makeHtml(text);
//
//   alert(html);
//
// Note: move the sample code to the bottom of this
// file before uncommenting it.
//

(function () {

    function identity(x) {
        return x;
    }
    function returnFalse(x) {
        return false;
    }

    function HookCollection() {
    }

    HookCollection.prototype = {

        chain: function (hookname, func) {
            var original = this[hookname];
            if (!original)
                throw new Error("unknown hook " + hookname);

            if (original === identity)
                this[hookname] = func;
            else
                this[hookname] = function (text) {
                    var args = Array.prototype.slice.call(arguments, 0);
                    args[0] = original.apply(null, args);
                    return func.apply(null, args);
                };
        },
        set: function (hookname, func) {
            if (!this[hookname])
                throw new Error("unknown hook " + hookname);
            this[hookname] = func;
        },
        addNoop: function (hookname) {
            this[hookname] = identity;
        },
        addFalse: function (hookname) {
            this[hookname] = returnFalse;
        }
    };

    Markdown.HookCollection = HookCollection;

    // g_urls and g_titles allow arbitrary user-entered strings as keys. This
    // caused an exception (and hence stopped the rendering) when the user entered
    // e.g. [push] or [__proto__]. Adding a prefix to the actual key prevents this
    // (since no builtin property starts with "s_"). See
    // http://meta.stackexchange.com/questions/64655/strange-wmd-bug
    // (granted, switching from Array() to Object() alone would have left only __proto__
    // to be a problem)
    function SaveHash() {
    }
    SaveHash.prototype = {
        set: function (key, value) {
            this["s_" + key] = value;
        },
        get: function (key) {
            return this["s_" + key];
        }
    };

    Markdown.Converter = function (OPTIONS) {
        var pluginHooks = this.hooks = new HookCollection();

        // given a URL that was encountered by itself (without markup), should return the link text that's to be given to this link
        pluginHooks.addNoop("plainLinkText");

        // called with the orignal text as given to makeHtml. The result of this plugin hook is the actual markdown source that will be cooked
        pluginHooks.addNoop("preConversion");

        // called with the text once all normalizations have been completed (tabs to spaces, line endings, etc.), but before any conversions have
        pluginHooks.addNoop("postNormalization");

        // Called with the text before / after creating block elements like code blocks and lists. Note that this is called recursively
        // with inner content, e.g. it's called with the full text, and then only with the content of a blockquote. The inner
        // call will receive outdented text.
        pluginHooks.addNoop("preBlockGamut");
        pluginHooks.addNoop("postBlockGamut");

        // called with the text of a single block element before / after the span-level conversions (bold, code spans, etc.) have been made
        pluginHooks.addNoop("preSpanGamut");
        pluginHooks.addNoop("postSpanGamut");

        // called with the final cooked HTML code. The result of this plugin hook is the actual output of makeHtml
        pluginHooks.addNoop("postConversion");

        //
        // Private state of the converter instance:
        //

        // Global hashes, used by various utility routines
        var g_urls;
        var g_titles;
        var g_html_blocks;

        // Used to track when we're inside an ordered or unordered list
        // (see _ProcessListItems() for details):
        var g_list_level;

        OPTIONS = OPTIONS || {};
        var asciify = identity, deasciify = identity;
        if (OPTIONS.nonAsciiLetters) {

            /* In JavaScript regular expressions, \w only denotes [a-zA-Z0-9_].
             * That's why there's inconsistent handling e.g. with intra-word bolding
             * of Japanese words. That's why we do the following if OPTIONS.nonAsciiLetters
             * is true:
             *
             * Before doing bold and italics, we find every instance
             * of a unicode word character in the Markdown source that is not
             * matched by \w, and the letter "Q". We take the character's code point
             * and encode it in base 51, using the "digits"
             *
             *     A, B, ..., P, R, ..., Y, Z, a, b, ..., y, z
             *
             * delimiting it with "Q" on both sides. For example, the source
             *
             * > In Chinese, the smurfs are called 藍精靈, meaning "blue spirits".
             *
             * turns into
             *
             * > In Chinese, the smurfs are called QNIhQQMOIQQOuUQ, meaning "blue spirits".
             *
             * Since everything that is a letter in Unicode is now a letter (or
             * several letters) in ASCII, \w and \b should always do the right thing.
             *
             * After the bold/italic conversion, we decode again; since "Q" was encoded
             * alongside all non-ascii characters (as "QBfQ"), and the conversion
             * will not generate "Q", the only instances of that letter should be our
             * encoded characters. And since the conversion will not break words, the
             * "Q...Q" should all still be in one piece.
             *
             * We're using "Q" as the delimiter because it's probably one of the
             * rarest characters, and also because I can't think of any special behavior
             * that would ever be triggered by this letter (to use a silly example, if we
             * delimited with "H" on the left and "P" on the right, then "Ψ" would be
             * encoded as "HTTP", which may cause special behavior). The latter would not
             * actually be a huge issue for bold/italic, but may be if we later use it
             * in other places as well.
             * */
            (function () {
                var lettersThatJavaScriptDoesNotKnowAndQ = /[Q\u00aa\u00b5\u00ba\u00c0-\u00d6\u00d8-\u00f6\u00f8-\u02c1\u02c6-\u02d1\u02e0-\u02e4\u02ec\u02ee\u0370-\u0374\u0376-\u0377\u037a-\u037d\u0386\u0388-\u038a\u038c\u038e-\u03a1\u03a3-\u03f5\u03f7-\u0481\u048a-\u0523\u0531-\u0556\u0559\u0561-\u0587\u05d0-\u05ea\u05f0-\u05f2\u0621-\u064a\u0660-\u0669\u066e-\u066f\u0671-\u06d3\u06d5\u06e5-\u06e6\u06ee-\u06fc\u06ff\u0710\u0712-\u072f\u074d-\u07a5\u07b1\u07c0-\u07ea\u07f4-\u07f5\u07fa\u0904-\u0939\u093d\u0950\u0958-\u0961\u0966-\u096f\u0971-\u0972\u097b-\u097f\u0985-\u098c\u098f-\u0990\u0993-\u09a8\u09aa-\u09b0\u09b2\u09b6-\u09b9\u09bd\u09ce\u09dc-\u09dd\u09df-\u09e1\u09e6-\u09f1\u0a05-\u0a0a\u0a0f-\u0a10\u0a13-\u0a28\u0a2a-\u0a30\u0a32-\u0a33\u0a35-\u0a36\u0a38-\u0a39\u0a59-\u0a5c\u0a5e\u0a66-\u0a6f\u0a72-\u0a74\u0a85-\u0a8d\u0a8f-\u0a91\u0a93-\u0aa8\u0aaa-\u0ab0\u0ab2-\u0ab3\u0ab5-\u0ab9\u0abd\u0ad0\u0ae0-\u0ae1\u0ae6-\u0aef\u0b05-\u0b0c\u0b0f-\u0b10\u0b13-\u0b28\u0b2a-\u0b30\u0b32-\u0b33\u0b35-\u0b39\u0b3d\u0b5c-\u0b5d\u0b5f-\u0b61\u0b66-\u0b6f\u0b71\u0b83\u0b85-\u0b8a\u0b8e-\u0b90\u0b92-\u0b95\u0b99-\u0b9a\u0b9c\u0b9e-\u0b9f\u0ba3-\u0ba4\u0ba8-\u0baa\u0bae-\u0bb9\u0bd0\u0be6-\u0bef\u0c05-\u0c0c\u0c0e-\u0c10\u0c12-\u0c28\u0c2a-\u0c33\u0c35-\u0c39\u0c3d\u0c58-\u0c59\u0c60-\u0c61\u0c66-\u0c6f\u0c85-\u0c8c\u0c8e-\u0c90\u0c92-\u0ca8\u0caa-\u0cb3\u0cb5-\u0cb9\u0cbd\u0cde\u0ce0-\u0ce1\u0ce6-\u0cef\u0d05-\u0d0c\u0d0e-\u0d10\u0d12-\u0d28\u0d2a-\u0d39\u0d3d\u0d60-\u0d61\u0d66-\u0d6f\u0d7a-\u0d7f\u0d85-\u0d96\u0d9a-\u0db1\u0db3-\u0dbb\u0dbd\u0dc0-\u0dc6\u0e01-\u0e30\u0e32-\u0e33\u0e40-\u0e46\u0e50-\u0e59\u0e81-\u0e82\u0e84\u0e87-\u0e88\u0e8a\u0e8d\u0e94-\u0e97\u0e99-\u0e9f\u0ea1-\u0ea3\u0ea5\u0ea7\u0eaa-\u0eab\u0ead-\u0eb0\u0eb2-\u0eb3\u0ebd\u0ec0-\u0ec4\u0ec6\u0ed0-\u0ed9\u0edc-\u0edd\u0f00\u0f20-\u0f29\u0f40-\u0f47\u0f49-\u0f6c\u0f88-\u0f8b\u1000-\u102a\u103f-\u1049\u1050-\u1055\u105a-\u105d\u1061\u1065-\u1066\u106e-\u1070\u1075-\u1081\u108e\u1090-\u1099\u10a0-\u10c5\u10d0-\u10fa\u10fc\u1100-\u1159\u115f-\u11a2\u11a8-\u11f9\u1200-\u1248\u124a-\u124d\u1250-\u1256\u1258\u125a-\u125d\u1260-\u1288\u128a-\u128d\u1290-\u12b0\u12b2-\u12b5\u12b8-\u12be\u12c0\u12c2-\u12c5\u12c8-\u12d6\u12d8-\u1310\u1312-\u1315\u1318-\u135a\u1380-\u138f\u13a0-\u13f4\u1401-\u166c\u166f-\u1676\u1681-\u169a\u16a0-\u16ea\u1700-\u170c\u170e-\u1711\u1720-\u1731\u1740-\u1751\u1760-\u176c\u176e-\u1770\u1780-\u17b3\u17d7\u17dc\u17e0-\u17e9\u1810-\u1819\u1820-\u1877\u1880-\u18a8\u18aa\u1900-\u191c\u1946-\u196d\u1970-\u1974\u1980-\u19a9\u19c1-\u19c7\u19d0-\u19d9\u1a00-\u1a16\u1b05-\u1b33\u1b45-\u1b4b\u1b50-\u1b59\u1b83-\u1ba0\u1bae-\u1bb9\u1c00-\u1c23\u1c40-\u1c49\u1c4d-\u1c7d\u1d00-\u1dbf\u1e00-\u1f15\u1f18-\u1f1d\u1f20-\u1f45\u1f48-\u1f4d\u1f50-\u1f57\u1f59\u1f5b\u1f5d\u1f5f-\u1f7d\u1f80-\u1fb4\u1fb6-\u1fbc\u1fbe\u1fc2-\u1fc4\u1fc6-\u1fcc\u1fd0-\u1fd3\u1fd6-\u1fdb\u1fe0-\u1fec\u1ff2-\u1ff4\u1ff6-\u1ffc\u203f-\u2040\u2054\u2071\u207f\u2090-\u2094\u2102\u2107\u210a-\u2113\u2115\u2119-\u211d\u2124\u2126\u2128\u212a-\u212d\u212f-\u2139\u213c-\u213f\u2145-\u2149\u214e\u2183-\u2184\u2c00-\u2c2e\u2c30-\u2c5e\u2c60-\u2c6f\u2c71-\u2c7d\u2c80-\u2ce4\u2d00-\u2d25\u2d30-\u2d65\u2d6f\u2d80-\u2d96\u2da0-\u2da6\u2da8-\u2dae\u2db0-\u2db6\u2db8-\u2dbe\u2dc0-\u2dc6\u2dc8-\u2dce\u2dd0-\u2dd6\u2dd8-\u2dde\u2e2f\u3005-\u3006\u3031-\u3035\u303b-\u303c\u3041-\u3096\u309d-\u309f\u30a1-\u30fa\u30fc-\u30ff\u3105-\u312d\u3131-\u318e\u31a0-\u31b7\u31f0-\u31ff\u3400-\u4db5\u4e00-\u9fc3\ua000-\ua48c\ua500-\ua60c\ua610-\ua62b\ua640-\ua65f\ua662-\ua66e\ua67f-\ua697\ua717-\ua71f\ua722-\ua788\ua78b-\ua78c\ua7fb-\ua801\ua803-\ua805\ua807-\ua80a\ua80c-\ua822\ua840-\ua873\ua882-\ua8b3\ua8d0-\ua8d9\ua900-\ua925\ua930-\ua946\uaa00-\uaa28\uaa40-\uaa42\uaa44-\uaa4b\uaa50-\uaa59\uac00-\ud7a3\uf900-\ufa2d\ufa30-\ufa6a\ufa70-\ufad9\ufb00-\ufb06\ufb13-\ufb17\ufb1d\ufb1f-\ufb28\ufb2a-\ufb36\ufb38-\ufb3c\ufb3e\ufb40-\ufb41\ufb43-\ufb44\ufb46-\ufbb1\ufbd3-\ufd3d\ufd50-\ufd8f\ufd92-\ufdc7\ufdf0-\ufdfb\ufe33-\ufe34\ufe4d-\ufe4f\ufe70-\ufe74\ufe76-\ufefc\uff10-\uff19\uff21-\uff3a\uff3f\uff41-\uff5a\uff66-\uffbe\uffc2-\uffc7\uffca-\uffcf\uffd2-\uffd7\uffda-\uffdc]/g;
                var cp_Q = "Q".charCodeAt(0);
                var cp_A = "A".charCodeAt(0);
                var cp_Z = "Z".charCodeAt(0);
                var dist_Za = "a".charCodeAt(0) - cp_Z - 1;

                asciify = function (text) {
                    return text.replace(lettersThatJavaScriptDoesNotKnowAndQ, function (m) {
                        var c = m.charCodeAt(0);
                        var s = "";
                        var v;
                        while (c > 0) {
                            v = (c % 51) + cp_A;
                            if (v >= cp_Q)
                                v++;
                            if (v > cp_Z)
                                v += dist_Za;
                            s = String.fromCharCode(v) + s;
                            c = c / 51 | 0;
                        }
                        return "Q" + s + "Q";
                    })
                };

                deasciify = function (text) {
                    return text.replace(/Q([A-PR-Za-z]{1,3})Q/g, function (m, s) {
                        var c = 0;
                        var v;
                        for (var i = 0; i < s.length; i++) {
                            v = s.charCodeAt(i);
                            if (v > cp_Z)
                                v -= dist_Za;
                            if (v > cp_Q)
                                v--;
                            v -= cp_A;
                            c = (c * 51) + v;
                        }
                        return String.fromCharCode(c);
                    })
                }
            })();
        }

        var _DoItalicsAndBold = OPTIONS.asteriskIntraWordEmphasis ? _DoItalicsAndBold_AllowIntrawordWithAsterisk : _DoItalicsAndBoldStrict;

        this.makeHtml = function (text) {

            //
            // Main function. The order in which other subs are called here is
            // essential. Link and image substitutions need to happen before
            // _EscapeSpecialCharsWithinTagAttributes(), so that any *'s or _'s in the <a>
            // and <img> tags get encoded.
            //

            // This will only happen if makeHtml on the same converter instance is called from a plugin hook.
            // Don't do that.
            if (g_urls)
                throw new Error("Recursive call to converter.makeHtml");

            // Create the private state objects.
            g_urls = new SaveHash();
            g_titles = new SaveHash();
            g_html_blocks = [];
            g_list_level = 0;

            text = pluginHooks.preConversion(text);

            // attacklab: Replace ~ with ~T
            // This lets us use tilde as an escape char to avoid md5 hashes
            // The choice of character is arbitray; anything that isn't
            // magic in Markdown will work.
            text = text.replace(/~/g, "~T");

            // attacklab: Replace $ with ~D
            // RegExp interprets $ as a special character
            // when it's in a replacement string
            text = text.replace(/\$/g, "~D");

            // Standardize line endings
            text = text.replace(/\r\n/g, "\n"); // DOS to Unix
            text = text.replace(/\r/g, "\n"); // Mac to Unix

            // Make sure text begins and ends with a couple of newlines:
            text = "\n\n" + text + "\n\n";

            // Convert all tabs to spaces.
            text = _Detab(text);

            // Strip any lines consisting only of spaces and tabs.
            // This makes subsequent regexen easier to write, because we can
            // match consecutive blank lines with /\n+/ instead of something
            // contorted like /[ \t]*\n+/ .
            text = text.replace(/^[ \t]+$/mg, "");

            text = pluginHooks.postNormalization(text);

            // Turn block-level HTML blocks into hash entries
            text = _HashHTMLBlocks(text);

            // Strip link definitions, store in hashes.
            text = _StripLinkDefinitions(text);

            text = _RunBlockGamut(text);

            text = _UnescapeSpecialChars(text);

            // attacklab: Restore dollar signs
            text = text.replace(/~D/g, "$$");

            // attacklab: Restore tildes
            text = text.replace(/~T/g, "~");

            text = pluginHooks.postConversion(text);

            g_html_blocks = g_titles = g_urls = null;

            return text;
        };

        function _StripLinkDefinitions(text) {
            //
            // Strips link definitions from text, stores the URLs and titles in
            // hash references.
            //

            // Link defs are in the form: ^[id]: url "optional title"

            /*
             text = text.replace(/
             ^[ ]{0,3}\[([^\[\]]+)\]:  // id = $1  attacklab: g_tab_width - 1
             [ \t]*
             \n?                 // maybe *one* newline
             [ \t]*
             <?(\S+?)>?          // url = $2
             (?=\s|$)            // lookahead for whitespace instead of the lookbehind removed below
             [ \t]*
             \n?                 // maybe one newline
             [ \t]*
             (                   // (potential) title = $3
             (\n*)           // any lines skipped = $4 attacklab: lookbehind removed
             [ \t]+
             ["(]
             (.+?)           // title = $5
             [")]
             [ \t]*
             )?                  // title is optional
             (\n+)             // subsequent newlines = $6, capturing because they must be put back if the potential title isn't an actual title
             /gm, function(){...});
             */

            text = text.replace(/^[ ]{0,3}\[([^\[\]]+)\]:[ \t]*\n?[ \t]*<?(\S+?)>?(?=\s|$)[ \t]*\n?[ \t]*((\n*)["(](.+?)[")][ \t]*)?(\n+)/gm,
                function (wholeMatch, m1, m2, m3, m4, m5, m6) {
                    m1 = m1.toLowerCase();
                    g_urls.set(m1, _EncodeAmpsAndAngles(m2));  // Link IDs are case-insensitive
                    if (m4) {
                        // Oops, found blank lines, so it's not a title.
                        // Put back the parenthetical statement we stole.
                        return m3 + m6;
                    } else if (m5) {
                        g_titles.set(m1, m5.replace(/"/g, "&quot;"));
                    }

                    // Completely remove the definition from the text
                    return "";
                }
            );

            return text;
        }

        function _HashHTMLBlocks(text) {

            // Hashify HTML blocks:
            // We only want to do this for block-level HTML tags, such as headers,
            // lists, and tables. That's because we still want to wrap <p>s around
            // "paragraphs" that are wrapped in non-block-level tags, such as anchors,
            // phrase emphasis, and spans. The list of tags we're looking for is
            // hard-coded:
            var block_tags_a = "p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math|ins|del"
            var block_tags_b = "p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math"

            // First, look for nested blocks, e.g.:
            //   <div>
            //     <div>
            //     tags for inner block must be indented.
            //     </div>
            //   </div>
            //
            // The outermost tags must start at the left margin for this to match, and
            // the inner nested divs must be indented.
            // We need to do this before the next, more liberal match, because the next
            // match will start at the first `<div>` and stop at the first `</div>`.

            // attacklab: This regex can be expensive when it fails.

            /*
             text = text.replace(/
             (                       // save in $1
             ^                   // start of line  (with /m)
             <($block_tags_a)    // start tag = $2
             \b                  // word break
             // attacklab: hack around khtml/pcre bug...
             [^\r]*?\n           // any number of lines, minimally matching
             </\2>               // the matching end tag
             [ \t]*              // trailing spaces/tabs
             (?=\n+)             // followed by a newline
             )                       // attacklab: there are sentinel newlines at end of document
             /gm,function(){...}};
             */
            text = text.replace(/^(<(p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math|ins|del)\b[^\r]*?\n<\/\2>[ \t]*(?=\n+))/gm, hashMatch);

            //
            // Now match more liberally, simply from `\n<tag>` to `</tag>\n`
            //

            /*
             text = text.replace(/
             (                       // save in $1
             ^                   // start of line  (with /m)
             <($block_tags_b)    // start tag = $2
             \b                  // word break
             // attacklab: hack around khtml/pcre bug...
             [^\r]*?             // any number of lines, minimally matching
             .*</\2>             // the matching end tag
             [ \t]*              // trailing spaces/tabs
             (?=\n+)             // followed by a newline
             )                       // attacklab: there are sentinel newlines at end of document
             /gm,function(){...}};
             */
            text = text.replace(/^(<(p|div|h[1-6]|blockquote|pre|table|dl|ol|ul|script|noscript|form|fieldset|iframe|math)\b[^\r]*?.*<\/\2>[ \t]*(?=\n+)\n)/gm, hashMatch);

            // Special case just for <hr />. It was easier to make a special case than
            // to make the other regex more complicated.

            /*
             text = text.replace(/
             \n                  // Starting after a blank line
             [ ]{0,3}
             (                   // save in $1
             (<(hr)          // start tag = $2
             \b          // word break
             ([^<>])*?
             \/?>)           // the matching end tag
             [ \t]*
             (?=\n{2,})      // followed by a blank line
             )
             /g,hashMatch);
             */
            text = text.replace(/\n[ ]{0,3}((<(hr)\b([^<>])*?\/?>)[ \t]*(?=\n{2,}))/g, hashMatch);

            // Special case for standalone HTML comments:

            /*
             text = text.replace(/
             \n\n                                            // Starting after a blank line
             [ ]{0,3}                                        // attacklab: g_tab_width - 1
             (                                               // save in $1
             <!
             (--(?:|(?:[^>-]|-[^>])(?:[^-]|-[^-])*)--)   // see http://www.w3.org/TR/html-markup/syntax.html#comments and http://meta.stackexchange.com/q/95256
             >
             [ \t]*
             (?=\n{2,})                                  // followed by a blank line
             )
             /g,hashMatch);
             */
            text = text.replace(/\n\n[ ]{0,3}(<!(--(?:|(?:[^>-]|-[^>])(?:[^-]|-[^-])*)--)>[ \t]*(?=\n{2,}))/g, hashMatch);

            // PHP and ASP-style processor instructions (<?...?> and <%...%>)

            /*
             text = text.replace(/
             (?:
             \n\n            // Starting after a blank line
             )
             (                   // save in $1
             [ ]{0,3}        // attacklab: g_tab_width - 1
             (?:
             <([?%])     // $2
             [^\r]*?
             \2>
             )
             [ \t]*
             (?=\n{2,})      // followed by a blank line
             )
             /g,hashMatch);
             */
            text = text.replace(/(?:\n\n)([ ]{0,3}(?:<([?%])[^\r]*?\2>)[ \t]*(?=\n{2,}))/g, hashMatch);

            return text;
        }

        function hashBlock(text) {
            text = text.replace(/(^\n+|\n+$)/g, "");
            // Replace the element text with a marker ("~KxK" where x is its key)
            return "\n\n~K" + (g_html_blocks.push(text) - 1) + "K\n\n";
        }

        function hashMatch(wholeMatch, m1) {
            return hashBlock(m1);
        }

        var blockGamutHookCallback = function (t) {
            return _RunBlockGamut(t);
        }

        function _RunBlockGamut(text, doNotUnhash, doNotCreateParagraphs) {
            //
            // These are all the transformations that form block-level
            // tags like paragraphs, headers, and list items.
            //

            text = pluginHooks.preBlockGamut(text, blockGamutHookCallback);

            text = _DoHeaders(text);

            // Do Horizontal Rules:
            var replacement = "<hr />\n";
            text = text.replace(/^[ ]{0,2}([ ]?\*[ ]?){3,}[ \t]*$/gm, replacement);
            text = text.replace(/^[ ]{0,2}([ ]?-[ ]?){3,}[ \t]*$/gm, replacement);
            text = text.replace(/^[ ]{0,2}([ ]?_[ ]?){3,}[ \t]*$/gm, replacement);

            text = _DoLists(text);
            text = _DoCodeBlocks(text);
            text = _DoBlockQuotes(text);

            text = pluginHooks.postBlockGamut(text, blockGamutHookCallback);

            // We already ran _HashHTMLBlocks() before, in Markdown(), but that
            // was to escape raw HTML in the original Markdown source. This time,
            // we're escaping the markup we've just created, so that we don't wrap
            // <p> tags around block-level tags.
            text = _HashHTMLBlocks(text);

            text = _FormParagraphs(text, doNotUnhash, doNotCreateParagraphs);

            return text;
        }

        function _RunSpanGamut(text) {
            //
            // These are all the transformations that occur *within* block-level
            // tags like paragraphs, headers, and list items.
            //

            text = pluginHooks.preSpanGamut(text);

            text = _DoCodeSpans(text);
            text = _EscapeSpecialCharsWithinTagAttributes(text);
            text = _EncodeBackslashEscapes(text);

            // Process anchor and image tags. Images must come first,
            // because ![foo][f] looks like an anchor.
            text = _DoImages(text);
            text = _DoAnchors(text);

            // Make links out of things like `<http://example.com/>`
            // Must come after _DoAnchors(), because you can use < and >
            // delimiters in inline links like [this](<url>).
            text = _DoAutoLinks(text);

            text = text.replace(/~P/g, "://"); // put in place to prevent autolinking; reset now

            text = _EncodeAmpsAndAngles(text);
            text = _DoItalicsAndBold(text);

            // Do hard breaks:
            text = text.replace(/  +\n/g, " <br>\n");

            text = pluginHooks.postSpanGamut(text);

            return text;
        }

        function _EscapeSpecialCharsWithinTagAttributes(text) {
            //
            // Within tags -- meaning between < and > -- encode [\ ` * _] so they
            // don't conflict with their use in Markdown for code, italics and strong.
            //

            // Build a regex to find HTML tags and comments.  See Friedl's
            // "Mastering Regular Expressions", 2nd Ed., pp. 200-201.

            // SE: changed the comment part of the regex

            var regex = /(<[a-z\/!$]("[^"]*"|'[^']*'|[^'">])*>|<!(--(?:|(?:[^>-]|-[^>])(?:[^-]|-[^-])*)--)>)/gi;

            text = text.replace(regex, function (wholeMatch) {
                var tag = wholeMatch.replace(/(.)<\/?code>(?=.)/g, "$1`");
                tag = escapeCharacters(tag, wholeMatch.charAt(1) == "!" ? "\\`*_/" : "\\`*_"); // also escape slashes in comments to prevent autolinking there -- http://meta.stackexchange.com/questions/95987
                return tag;
            });

            return text;
        }

        function _DoAnchors(text) {

            if (text.indexOf("[") === -1)
                return text;

            //
            // Turn Markdown link shortcuts into XHTML <a> tags.
            //
            //
            // First, handle reference-style links: [link text] [id]
            //

            /*
             text = text.replace(/
             (                           // wrap whole match in $1
             \[
             (
             (?:
             \[[^\]]*\]      // allow brackets nested one level
             |
             [^\[]           // or anything else
             )*
             )
             \]
             [ ]?                    // one optional space
             (?:\n[ ]*)?             // one optional newline followed by spaces
             \[
             (.*?)                   // id = $3
             \]
             )
             ()()()()                    // pad remaining backreferences
             /g, writeAnchorTag);
             */
            text = text.replace(/(\[((?:\[[^\]]*\]|[^\[\]])*)\][ ]?(?:\n[ ]*)?\[(.*?)\])()()()()/g, writeAnchorTag);

            //
            // Next, inline-style links: [link text](url "optional title")
            //

            /*
             text = text.replace(/
             (                           // wrap whole match in $1
             \[
             (
             (?:
             \[[^\]]*\]      // allow brackets nested one level
             |
             [^\[\]]         // or anything else
             )*
             )
             \]
             \(                      // literal paren
             [ \t]*
             ()                      // no id, so leave $3 empty
             <?(                     // href = $4
             (?:
             \([^)]*\)       // allow one level of (correctly nested) parens (think MSDN)
             |
             [^()\s]
             )*?
             )>?
             [ \t]*
             (                       // $5
             (['"])              // quote char = $6
             (.*?)               // Title = $7
             \6                  // matching quote
             [ \t]*              // ignore any spaces/tabs between closing quote and )
             )?                      // title is optional
             \)
             )
             /g, writeAnchorTag);
             */

            text = text.replace(/(\[((?:\[[^\]]*\]|[^\[\]])*)\]\([ \t]*()<?((?:\([^)]*\)|[^()\s])*?)>?[ \t]*((['"])(.*?)\6[ \t]*)?\))/g, writeAnchorTag);

            //
            // Last, handle reference-style shortcuts: [link text]
            // These must come last in case you've also got [link test][1]
            // or [link test](/foo)
            //

            /*
             text = text.replace(/
             (                   // wrap whole match in $1
             \[
             ([^\[\]]+)      // link text = $2; can't contain '[' or ']'
             \]
             )
             ()()()()()          // pad rest of backreferences
             /g, writeAnchorTag);
             */
            text = text.replace(/(\[([^\[\]]+)\])()()()()()/g, writeAnchorTag);

            return text;
        }

        function writeAnchorTag(wholeMatch, m1, m2, m3, m4, m5, m6, m7) {
            if (m7 == undefined) m7 = "";
            var whole_match = m1;
            var link_text = m2.replace(/:\/\//g, "~P"); // to prevent auto-linking withing the link. will be converted back after the auto-linker runs
            var link_id = m3.toLowerCase();
            var url = m4;
            var title = m7;

            if (url == "") {
                if (link_id == "") {
                    // lower-case and turn embedded newlines into spaces
                    link_id = link_text.toLowerCase().replace(/ ?\n/g, " ");
                }
                url = "#" + link_id;

                if (g_urls.get(link_id) != undefined) {
                    url = g_urls.get(link_id);
                    if (g_titles.get(link_id) != undefined) {
                        title = g_titles.get(link_id);
                    }
                }
                else {
                    if (whole_match.search(/\(\s*\)$/m) > -1) {
                        // Special case for explicit empty url
                        url = "";
                    } else {
                        return whole_match;
                    }
                }
            }
            url = attributeSafeUrl(url);

            var result = "<a href=\"" + url + "\"";

            if (title != "") {
                title = attributeEncode(title);
                title = escapeCharacters(title, "*_");
                result += " title=\"" + title + "\"";
            }

            result += ">" + link_text + "</a>";

            return result;
        }

        function _DoImages(text) {

            if (text.indexOf("![") === -1)
                return text;

            //
            // Turn Markdown image shortcuts into <img> tags.
            //

            //
            // First, handle reference-style labeled images: ![alt text][id]
            //

            /*
             text = text.replace(/
             (                   // wrap whole match in $1
             !\[
             (.*?)           // alt text = $2
             \]
             [ ]?            // one optional space
             (?:\n[ ]*)?     // one optional newline followed by spaces
             \[
             (.*?)           // id = $3
             \]
             )
             ()()()()            // pad rest of backreferences
             /g, writeImageTag);
             */
            text = text.replace(/(!\[(.*?)\][ ]?(?:\n[ ]*)?\[(.*?)\])()()()()/g, writeImageTag);

            //
            // Next, handle inline images:  ![alt text](url "optional title")
            // Don't forget: encode * and _

            /*
             text = text.replace(/
             (                   // wrap whole match in $1
             !\[
             (.*?)           // alt text = $2
             \]
             \s?             // One optional whitespace character
             \(              // literal paren
             [ \t]*
             ()              // no id, so leave $3 empty
             <?(\S+?)>?      // src url = $4
             [ \t]*
             (               // $5
             (['"])      // quote char = $6
             (.*?)       // title = $7
             \6          // matching quote
             [ \t]*
             )?              // title is optional
             \)
             )
             /g, writeImageTag);
             */
            text = text.replace(/(!\[(.*?)\]\s?\([ \t]*()<?(\S+?)>?[ \t]*((['"])(.*?)\6[ \t]*)?\))/g, writeImageTag);

            return text;
        }

        function attributeEncode(text) {
            // unconditionally replace angle brackets here -- what ends up in an attribute (e.g. alt or title)
            // never makes sense to have verbatim HTML in it (and the sanitizer would totally break it)
            return text.replace(/>/g, "&gt;").replace(/</g, "&lt;").replace(/"/g, "&quot;").replace(/'/g, "&#39;");
        }

        function writeImageTag(wholeMatch, m1, m2, m3, m4, m5, m6, m7) {
            var whole_match = m1;
            var alt_text = m2;
            var link_id = m3.toLowerCase();
            var url = m4;
            var title = m7;

            if (!title) title = "";

            if (url == "") {
                if (link_id == "") {
                    // lower-case and turn embedded newlines into spaces
                    link_id = alt_text.toLowerCase().replace(/ ?\n/g, " ");
                }
                url = "#" + link_id;

                if (g_urls.get(link_id) != undefined) {
                    url = g_urls.get(link_id);
                    if (g_titles.get(link_id) != undefined) {
                        title = g_titles.get(link_id);
                    }
                }
                else {
                    return whole_match;
                }
            }

            alt_text = escapeCharacters(attributeEncode(alt_text), "*_[]()");
            url = escapeCharacters(url, "*_");
            var result = "<img src=\"" + url + "\" alt=\"" + alt_text + "\"";

            // attacklab: Markdown.pl adds empty title attributes to images.
            // Replicate this bug.

            //if (title != "") {
            title = attributeEncode(title);
            title = escapeCharacters(title, "*_");
            result += " title=\"" + title + "\"";
            //}

            result += " />";

            return result;
        }

        function _DoHeaders(text) {

            // Setext-style headers:
            //  Header 1
            //  ========
            //
            //  Header 2
            //  --------
            //
            text = text.replace(/^(.+)[ \t]*\n=+[ \t]*\n+/gm,
                function (wholeMatch, m1) {
                    return "<h1>" + _RunSpanGamut(m1) + "</h1>\n\n";
                }
            );

            text = text.replace(/^(.+)[ \t]*\n-+[ \t]*\n+/gm,
                function (matchFound, m1) {
                    return "<h2>" + _RunSpanGamut(m1) + "</h2>\n\n";
                }
            );

            // atx-style headers:
            //  # Header 1
            //  ## Header 2
            //  ## Header 2 with closing hashes ##
            //  ...
            //  ###### Header 6
            //

            /*
             text = text.replace(/
             ^(\#{1,6})      // $1 = string of #'s
             [ \t]*
             (.+?)           // $2 = Header text
             [ \t]*
             \#*             // optional closing #'s (not counted)
             \n+
             /gm, function() {...});
             */

            text = text.replace(/^(\#{1,6})[ \t]*(.+?)[ \t]*\#*\n+/gm,
                function (wholeMatch, m1, m2) {
                    var h_level = m1.length;
                    return "<h" + h_level + ">" + _RunSpanGamut(m2) + "</h" + h_level + ">\n\n";
                }
            );

            return text;
        }

        function _DoLists(text, isInsideParagraphlessListItem) {
            //
            // Form HTML ordered (numbered) and unordered (bulleted) lists.
            //

            // attacklab: add sentinel to hack around khtml/safari bug:
            // http://bugs.webkit.org/show_bug.cgi?id=11231
            text += "~0";

            // Re-usable pattern to match any entirel ul or ol list:

            /*
             var whole_list = /
             (                                   // $1 = whole list
             (                               // $2
             [ ]{0,3}                    // attacklab: g_tab_width - 1
             ([*+-]|\d+[.])              // $3 = first list item marker
             [ \t]+
             )
             [^\r]+?
             (                               // $4
             ~0                          // sentinel for workaround; should be $
             |
             \n{2,}
             (?=\S)
             (?!                         // Negative lookahead for another list item marker
             [ \t]*
             (?:[*+-]|\d+[.])[ \t]+
             )
             )
             )
             /g
             */
            var whole_list = /^(([ ]{0,3}([*+-]|\d+[.])[ \t]+)[^\r]*?(~0|\n{2,}(?=\S)(?![ \t]*(?:[*+-]|\d+[.])[ \t]+)))/gm;
            var list_type;
            if (g_list_level) {
                text = text.replace(whole_list, function (wholeMatch, m1, m2) {
                    var list = m1;
                    list_type = getListType(m2);
                    //2015-10-22 wiz：删除起始序列号 支持
                    //var first_number;
                    //if (list_type === "ol")
                    //    first_number = parseInt(m2, 10)

                    var result = _ProcessListItems(list, list_type, isInsideParagraphlessListItem);

                    // Trim any trailing whitespace, to put the closing `</$list_type>`
                    // up on the preceding line, to get it past the current stupid
                    // HTML block parser. This is a hack to work around the terrible
                    // hack that is the HTML block parser.
                    var resultStr = result.list_str.replace(/\s+$/, "");
                    var opening = "<" + list_type;
                    //if (first_number && first_number !== 1)
                    //    opening += " start=\"" + first_number + "\"";
                    resultStr = opening + ">" + resultStr + "</" + result.list_type + ">\n\n";
                    list_type = result.list_type;
                    return resultStr;
                });
            } else {
                whole_list = /(\n\n|^\n?)(([ ]{0,3}([*+-]|\d+[.])[ \t]+)[^\r]+?(~0|\n{2,}(?=\S)(?![ \t]*(?:[*+-]|\d+[.])[ \t]+)))/gm;
                text = text.replace(whole_list, function (wholeMatch, m1, m2, m3) {
                    var runup = m1;
                    var list = m2;
                    list_type = getListType(m3);
                    //2015-10-22 wiz：删除起始序列号 支持
                    //var first_number;
                    //if (list_type === "ol")
                    //    first_number = parseInt(m3, 10)

                    var result = _ProcessListItems(list, list_type);

                    var opening = "<" + list_type;
                    //if (first_number && first_number !== 1)
                    //    opening += " start=\"" + first_number + "\"";

                    var resultStr = runup + opening + ">\n" + result.list_str + "</" + result.list_type + ">\n\n";
                    list_type = result.list_type;
                    return resultStr;
                });
            }

            // attacklab: strip sentinel
            text = text.replace(/~0/, "");

            return text;
        }

        var _listItemMarkers = {ol: "\\d+[.]", ul: "[*+-]"};

        function getListType(str) {
            return (str.search(/[*+-]/g) > -1) ? "ul" : "ol";
        }

        function _ProcessListItems(list_str, list_type, isInsideParagraphlessListItem) {
            //
            //  Process the contents of a single ordered or unordered list, splitting it
            //  into individual list items.
            //
            //  list_type is either "ul" or "ol".

            // The $g_list_level global keeps track of when we're inside a list.
            // Each time we enter a list, we increment it; when we leave a list,
            // we decrement. If it's zero, we're not in a list anymore.
            //
            // We do this because when we're not inside a list, we want to treat
            // something like this:
            //
            //    I recommend upgrading to version
            //    8. Oops, now this line is treated
            //    as a sub-list.
            //
            // As a single paragraph, despite the fact that the second line starts
            // with a digit-period-space sequence.
            //
            // Whereas when we're inside a list (or sub-list), that line will be
            // treated as the start of a sub-list. What a kludge, huh? This is
            // an aspect of Markdown's syntax that's hard to parse perfectly
            // without resorting to mind-reading. Perhaps the solution is to
            // change the syntax rules such that sub-lists must start with a
            // starting cardinal number; e.g. "1." or "a.".

            g_list_level++;

            // trim trailing blank lines:
            list_str = list_str.replace(/\n{2,}$/, "\n");

            // attacklab: add sentinel to emulate \z
            list_str += "~0";

            // In the original attacklab showdown, list_type was not given to this function, and anything
            // that matched /[*+-]|\d+[.]/ would just create the next <li>, causing this mismatch:
            //
            //  Markdown          rendered by WMD        rendered by MarkdownSharp
            //  ------------------------------------------------------------------
            //  1. first          1. first               1. first
            //  2. second         2. second              2. second
            //  - third           3. third                   * third
            //
            // We changed this to behave identical to MarkdownSharp. This is the constructed RegEx,
            // with {MARKER} being one of \d+[.] or [*+-], depending on list_type:

            /*
             list_str = list_str.replace(/
             (^[ \t]*)                       // leading whitespace = $1
             ({MARKER}) [ \t]+               // list marker = $2
             ([^\r]+?                        // list item text   = $3
             (\n+)
             )
             (?=
             (~0 | \2 ({MARKER}) [ \t]+)
             )
             /gm, function(){...});
             */

            //2015-10-22 wiz: 修改 list 的支持规则， 同级的 无序列表 和 有序列表 不会自动处理为 父子关系， 而是生成平级的两个列表；
            //var marker = _listItemMarkers[list_type];
            //var re = new RegExp("(^[ \\t]*)(" + marker + ")[ \\t]+([^\\r]+?(\\n+))(?=(~0|\\1(" + marker + ")[ \\t]+))", "gm");
            var re = new RegExp("(^[ \\t]*)([*+-]|\\d+[.])[ \\t]+([^\\r]*?(\\n+))(?=(~0|\\1([*+-]|\\d+[.])[ \\t]+))", "gm");
            var last_item_had_a_double_newline = false;
            list_str = list_str.replace(re,
                function (wholeMatch, m1, m2, m3) {
                    var item = m3;
                    var leading_space = m1;
                    var cur_list_type = getListType(m2);
                    var ends_with_double_newline = /\n\n$/.test(item);
                    var contains_double_newline = ends_with_double_newline || item.search(/\n{2,}/) > -1;
                    var isTodo = /^\[( |x)\]/.test(item);

                    var loose = contains_double_newline || last_item_had_a_double_newline;
                    item = _RunBlockGamut(_Outdent(item), /* doNotUnhash = */true, /* doNotCreateParagraphs = */ !loose);

                    var itemHtml = '';
                    if (cur_list_type != list_type) {
                        itemHtml = '</' + list_type + '>\n<' + cur_list_type + '>\n';
                        list_type = cur_list_type;
                    }
                    // 判断是否为 todo_list
                    if (isTodo) {
                        itemHtml += "<li class='wiz-md-todo-list-item'>";
                    } else {
                        itemHtml += "<li>";
                    }
                    itemHtml += (item + "</li>\n");

                    last_item_had_a_double_newline = ends_with_double_newline;
                    return itemHtml;
                }
            );

            // attacklab: strip sentinel
            list_str = list_str.replace(/~0/g, "");

            g_list_level--;
            return {list_str: list_str, list_type: list_type};
        }

        function _DoCodeBlocks(text) {
            //
            //  Process Markdown `<pre><code>` blocks.
            //

            /*
             text = text.replace(/
             (?:\n\n|^)
             (                               // $1 = the code block -- one or more lines, starting with a space/tab
             (?:
             (?:[ ]{4}|\t)           // Lines must start with a tab or a tab-width of spaces - attacklab: g_tab_width
             .*\n+
             )+
             )
             (\n*[ ]{0,3}[^ \t\n]|(?=~0))    // attacklab: g_tab_width
             /g ,function(){...});
             */

            // attacklab: sentinel workarounds for lack of \A and \Z, safari\khtml bug
            text += "~0";

            text = text.replace(/(?:\n\n|^\n?)((?:(?:[ ]{4}|\t).*\n+)+)(\n*[ ]{0,3}[^ \t\n]|(?=~0))/g,
                function (wholeMatch, m1, m2) {
                    var codeblock = m1;
                    var nextChar = m2;

                    codeblock = _EncodeCode(_Outdent(codeblock));
                    codeblock = _Detab(codeblock);
                    codeblock = codeblock.replace(/^\n+/g, ""); // trim leading newlines
                    codeblock = codeblock.replace(/\n+$/g, ""); // trim trailing whitespace

                    codeblock = "<pre><code>" + codeblock + "\n</code></pre>";

                    return "\n\n" + codeblock + "\n\n" + nextChar;
                }
            );

            // attacklab: strip sentinel
            text = text.replace(/~0/, "");

            return text;
        }

        function _DoCodeSpans(text) {
            //
            // * Backtick quotes are used for <code></code> spans.
            //
            // * You can use multiple backticks as the delimiters if you want to
            //   include literal backticks in the code span. So, this input:
            //
            //      Just type ``foo `bar` baz`` at the prompt.
            //
            //   Will translate to:
            //
            //      <p>Just type <code>foo `bar` baz</code> at the prompt.</p>
            //
            //   There's no arbitrary limit to the number of backticks you
            //   can use as delimters. If you need three consecutive backticks
            //   in your code, use four for delimiters, etc.
            //
            // * You can use spaces to get literal backticks at the edges:
            //
            //      ... type `` `bar` `` ...
            //
            //   Turns to:
            //
            //      ... type <code>`bar`</code> ...
            //

            /*
             text = text.replace(/
             (^|[^\\`])      // Character before opening ` can't be a backslash or backtick
             (`+)            // $2 = Opening run of `
             (?!`)           // and no more backticks -- match the full run
             (               // $3 = The code block
             [^\r]*?
             [^`]        // attacklab: work around lack of lookbehind
             )
             \2              // Matching closer
             (?!`)
             /gm, function(){...});
             */

            text = text.replace(/(^|[^\\`])(`+)(?!`)([^\r]*?[^`])\2(?!`)/gm,
                function (wholeMatch, m1, m2, m3, m4) {
                    var c = m3;
                    c = c.replace(/^([ \t]*)/g, ""); // leading whitespace
                    c = c.replace(/[ \t]*$/g, ""); // trailing whitespace
                    c = _EncodeCode(c);
                    c = c.replace(/:\/\//g, "~P"); // to prevent auto-linking. Not necessary in code *blocks*, but in code spans. Will be converted back after the auto-linker runs.
                    return m1 + "<code>" + c + "</code>";
                }
            );

            return text;
        }

        function _EncodeCode(text) {
            //
            // Encode/escape certain characters inside Markdown code runs.
            // The point is that in code, these characters are literals,
            // and lose their special Markdown meanings.
            //
            // Encode all ampersands; HTML entities are not
            // entities within a Markdown code span.
            text = text.replace(/&/g, "&amp;");

            // Do the angle bracket song and dance:
            text = text.replace(/</g, "&lt;");
            text = text.replace(/>/g, "&gt;");

            // Now, escape characters that are magic in Markdown:
            text = escapeCharacters(text, "\*_{}[]\\", false);

            // jj the line above breaks this:
            //---

            //* Item

            //   1. Subitem

            //            special char: *
            //---

            return text;
        }

        function _DoItalicsAndBoldStrict(text) {

            if (text.indexOf("*") === -1 && text.indexOf("_") === -1)
                return text;

            text = asciify(text);

            // <strong> must go first:

            // (^|[\W_])           Start with a non-letter or beginning of string. Store in \1.
            // (?:(?!\1)|(?=^))    Either the next character is *not* the same as the previous,
            //                     or we started at the end of the string (in which case the previous
            //                     group had zero width, so we're still there). Because the next
            //                     character is the marker, this means that if there are e.g. multiple
            //                     underscores in a row, we can only match the left-most ones (which
            //                     prevents foo___bar__ from getting bolded)
            // (\*|_)              The marker character itself, asterisk or underscore. Store in \2.
            // \2                  The marker again, since bold needs two.
            // (?=\S)              The first bolded character cannot be a space.
            // ([^\r]*?\S)         The actual bolded string. At least one character, and it cannot *end*
            //                     with a space either. Note that like in many other places, [^\r] is
            //                     just a workaround for JS' lack of single-line regexes; it's equivalent
            //                     to a . in an /s regex, because the string cannot contain any \r (they
            //                     are removed in the normalizing step).
            // \2\2                The marker character, twice -- end of bold.
            // (?!\2)              Not followed by another marker character (ensuring that we match the
            //                     rightmost two in a longer row)...
            // (?=[\W_]|$)         ...but by any other non-word character or the end of string.
            text = text.replace(/(^|[\W_])(?:(?!\1)|(?=^))(\*|_)\2(?=\S)([^\r]*?\S)\2\2(?!\2)(?=[\W_]|$)/g,
                "$1<strong>$3</strong>");

            // This is almost identical to the <strong> regex, except 1) there's obviously just one marker
            // character, and 2) the italicized string cannot contain the marker character.
            text = text.replace(/(^|[\W_])(?:(?!\1)|(?=^))(\*|_)(?=\S)((?:(?!\2)[^\r])*?\S)\2(?!\2)(?=[\W_]|$)/g,
                "$1<em>$3</em>");

            return deasciify(text);
        }

        function _DoItalicsAndBold_AllowIntrawordWithAsterisk(text) {

            if (text.indexOf("*") === -1 && text.indexOf("_") === -1)
                return text;

            text = asciify(text);

            // <strong> must go first:
            // (?=[^\r][*_]|[*_])               Optimization only, to find potentially relevant text portions faster. Minimally slower in Chrome, but much faster in IE.
            // (                                Store in \1. This is the last character before the delimiter
            //     ^                            Either we're at the start of the string (i.e. there is no last character)...
            //     |                            ... or we allow one of the following:
            //     (?=                          (lookahead; we're not capturing this, just listing legal possibilities)
            //         \W__                     If the delimiter is __, then this last character must be non-word non-underscore (extra-word emphasis only)
            //         |
            //         (?!\*)[\W_]\*\*          If the delimiter is **, then this last character can be non-word non-asterisk (extra-word emphasis)...
            //         |
            //         \w\*\*\w                 ...or it can be word/underscore, but only if the first bolded character is such a character as well (intra-word emphasis)
            //     )
            //     [^\r]                        actually capture the character (can't use `.` since it could be \n)
            // )
            // (\*\*|__)                        Store in \2: the actual delimiter
            // (?!\2)                           not followed by the delimiter again (at most one more asterisk/underscore is allowed)
            // (?=\S)                           the first bolded character can't be a space
            // (                                Store in \3: the bolded string
            //
            //     (?:|                         Look at all bolded characters except for the last one. Either that's empty, meaning only a single character was bolded...
            //       [^\r]*?                    ... otherwise take arbitrary characters, minimally matching; that's all bolded characters except for the last *two*
            //       (?!\2)                       the last two characters cannot be the delimiter itself (because that would mean four underscores/asterisks in a row)
            //       [^\r]                        capture the next-to-last bolded character
            //     )
            //     (?=                          lookahead at the very last bolded char and what comes after
            //         \S_                      for underscore-bolding, it can be any non-space
            //         |
            //         \w                       for asterisk-bolding (otherwise the previous alternative would've matched, since \w implies \S), either the last char is word/underscore...
            //         |
            //         \S\*\*(?:[\W_]|$)        ... or it's any other non-space, but in that case the character *after* the delimiter may not be a word character
            //     )
            //     .                            actually capture the last character (can use `.` this time because the lookahead ensures \S in all cases)
            // )
            // (?=                              lookahead; list the legal possibilities for the closing delimiter and its following character
            //     __(?:\W|$)                   for underscore-bolding, the following character (if any) must be non-word non-underscore
            //     |
            //     \*\*(?:[^*]|$)               for asterisk-bolding, any non-asterisk is allowed (note we already ensured above that it's not a word character if the last bolded character wasn't one)
            // )
            // \2                               actually capture the closing delimiter (and make sure that it matches the opening one)


            //2015-10-26 改善对 xxx**(1)**xxx 的支持
            //text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W__|(?!\*)[\W_]\*\*|\w\*\*\w)[^\r])(\*\*|__)(?!\2)(?=\S)((?:|[^\r]*?(?!\2)[^\r])(?=\S_|\w|\S\*\*(?:[\W_]|$)).)(?=__(?:\W|$)|\*\*(?:[^*]|$))\2/g,
            //    "$1<strong>$3</strong>");
            text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W__|(?!\*)[\w\W_]\*\*|\w\*\*\w)[^\r])(\*\*|__)(?!\2)(?=\S)((?:|[^\r]*?(?!\2)[^\r])(?=\S_|\w|.\*\*(?:[\w\W_]|$)).)(?=__(?:\W|$)|\*\*(?:[^*]|$))\2/g,
                "$1<strong>$3</strong>");

            // now <em>:
            // (?=[^\r][*_]|[*_])               Optimization, see above.
            // (                                Store in \1. This is the last character before the delimiter
            //     ^                            Either we're at the start of the string (i.e. there is no last character)...
            //     |                            ... or we allow one of the following:
            //     (?=                          (lookahead; we're not capturing this, just listing legal possibilities)
            //         \W_                      If the delimiter is _, then this last character must be non-word non-underscore (extra-word emphasis only)
            //         |
            //         (?!\*)                   otherwise, we list two possiblities for * as the delimiter; in either case, the last characters cannot be an asterisk itself
            //         (?:
            //             [\W_]\*              this last character can be non-word (extra-word emphasis)...
            //             |
            //             \D\*(?=\w)\D         ...or it can be word (otherwise the first alternative would've matched), but only if
            //                                      a) the first italicized character is such a character as well (intra-word emphasis), and
            //                                      b) neither character on either side of the asterisk is a digit
            //         )
            //     )
            //     [^\r]                        actually capture the character (can't use `.` since it could be \n)
            // )
            // (\*|_)                           Store in \2: the actual delimiter
            // (?!\2\2\2)                       not followed by more than two more instances of the delimiter
            // (?=\S)                           the first italicized character can't be a space
            // (                                Store in \3: the italicized string
            //     (?:(?!\2)[^\r])*?            arbitrary characters except for the delimiter itself, minimally matching
            //     (?=                          lookahead at the very last italicized char and what comes after
            //         [^\s_]_                  for underscore-italicizing, it can be any non-space non-underscore
            //         |
            //         (?=\w)\D\*\D             for asterisk-italicizing, either the last char is word/underscore *and* neither character on either side of the asterisk is a digit...
            //         |
            //         [^\s*]\*(?:[\W_]|$)      ... or that last char is any other non-space non-asterisk, but then the character after the delimiter (if any) must be non-word
            //     )
            //     .                            actually capture the last character (can use `.` this time because the lookahead ensures \S in all cases)
            // )
            // (?=                              lookahead; list the legal possibilities for the closing delimiter and its following character
            //     _(?:\W|$)                    for underscore-italicizing, the following character (if any) must be non-word non-underscore
            //     |
            //     \*(?:[^*]|$)                 for asterisk-italicizing, any non-asterisk is allowed; all other restrictions have already been ensured in the previous lookahead
            // )
            // \2                               actually capture the closing delimiter (and make sure that it matches the opening one)

            //2015-10-26 改善对 xxx*(1)*xxx 的支持
            //text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W_|(?!\*)(?:[\W_]\*|\D\*(?=\w)\D))[^\r])(\*|_)(?!\2\2\2)(?=\S)((?:(?!\2)[^\r])*?(?=[^\s_]_|(?=\w)\D\*\D|[^\s*]\*(?:[\W_]|$)).)(?=_(?:\W|$)|\*(?:[^*]|$))\2/g,
            //    "$1<em>$3</em>");
            text = text.replace(/(?=[^\r][*_]|[*_])(^|(?=\W_|(?!\*)(?:[\w\W_]\*|\D\*(?=\w)\D))[^\r])(\*|_)(?!\2\2\2)(?=\S)((?:(?!\2)[^\r])*?(?=[^\s_]_|(?=[\w\W])\D\*\D|[^\s*]\*(?:[\w\W_]|$)).)(?=_(?:\W|$)|\*(?:[^*]|$))\2/g,
                "$1<em>$3</em>");

            return deasciify(text);
        }


        function _DoBlockQuotes(text) {

            /*
             text = text.replace(/
             (                           // Wrap whole match in $1
             (
             ^[ \t]*>[ \t]?      // '>' at the start of a line
             .+\n                // rest of the first line
             (.+\n)*             // subsequent consecutive lines
             \n*                 // blanks
             )+
             )
             /gm, function(){...});
             */

            text = text.replace(/((^[ \t]*>[ \t]?.+\n(.+\n)*\n*)+)/gm,
                function (wholeMatch, m1) {
                    var bq = m1;

                    // attacklab: hack around Konqueror 3.5.4 bug:
                    // "----------bug".replace(/^-/g,"") == "bug"

                    bq = bq.replace(/^[ \t]*>[ \t]?/gm, "~0"); // trim one level of quoting

                    // attacklab: clean up hack
                    bq = bq.replace(/~0/g, "");

                    bq = bq.replace(/^[ \t]+$/gm, "");     // trim whitespace-only lines
                    bq = _RunBlockGamut(bq);             // recurse

                    bq = bq.replace(/(^|\n)/g, "$1  ");
                    // These leading spaces screw with <pre> content, so we need to fix that:
                    bq = bq.replace(
                        /(\s*<pre>[^\r]+?<\/pre>)/gm,
                        function (wholeMatch, m1) {
                            var pre = m1;
                            // attacklab: hack around Konqueror 3.5.4 bug:
                            pre = pre.replace(/^  /mg, "~0");
                            pre = pre.replace(/~0/g, "");
                            return pre;
                        });

                    return hashBlock("<blockquote>\n" + bq + "\n</blockquote>");
                }
            );
            return text;
        }

        function _FormParagraphs(text, doNotUnhash, doNotCreateParagraphs) {
            //
            //  Params:
            //    $text - string to process with html <p> tags
            //

            // Strip leading and trailing lines:
            text = text.replace(/^\n+/g, "");
            text = text.replace(/\n+$/g, "");

            var grafs = text.split(/\n{2,}/g);
            var grafsOut = [];

            var markerRe = /~K(\d+)K/;

            //
            // Wrap <p> tags.
            //
            var end = grafs.length;
            for (var i = 0; i < end; i++) {
                var str = grafs[i];

                // if this is an HTML marker, copy it
                if (markerRe.test(str)) {
                    grafsOut.push(str);
                }
                else if (/\S/.test(str)) {
                    str = _RunSpanGamut(str);
                    str = str.replace(/^([ \t]*)/g, doNotCreateParagraphs ? "" : "<p>");
                    if (!doNotCreateParagraphs)
                        str += "</p>"
                    grafsOut.push(str);
                }

            }
            //
            // Unhashify HTML blocks
            //
            if (!doNotUnhash) {
                end = grafsOut.length;
                for (var i = 0; i < end; i++) {
                    var foundAny = true;
                    while (foundAny) { // we may need several runs, since the data may be nested
                        foundAny = false;
                        grafsOut[i] = grafsOut[i].replace(/~K(\d+)K/g, function (wholeMatch, id) {
                            foundAny = true;
                            return g_html_blocks[id];
                        });
                    }
                }
            }
            return grafsOut.join("\n\n");
        }

        function _EncodeAmpsAndAngles(text) {
            // Smart processing for ampersands and angle brackets that need to be encoded.

            // Ampersand-encoding based entirely on Nat Irons's Amputator MT plugin:
            //   http://bumppo.net/projects/amputator/
            text = text.replace(/&(?!#?[xX]?(?:[0-9a-fA-F]+|\w+);)/g, "&amp;");

            // Encode naked <'s
            // support x<<y
            text = text.replace(/<(?!([a-z\/?!][^<>]*>)|~D)/gi, "&lt;");

            return text;
        }

        function _EncodeBackslashEscapes(text) {
            //
            //   Parameter:  String.
            //   Returns:    The string, with after processing the following backslash
            //               escape sequences.
            //

            // attacklab: The polite way to do this is with the new
            // escapeCharacters() function:
            //
            //     text = escapeCharacters(text,"\\",true);
            //     text = escapeCharacters(text,"`*_{}[]()>#+-.!",true);
            //
            // ...but we're sidestepping its use of the (slow) RegExp constructor
            // as an optimization for Firefox.  This function gets called a LOT.

            text = text.replace(/\\(\\)/g, escapeCharacters_callback);
            text = text.replace(/\\([`*_{}\[\]()>#+-.!])/g, escapeCharacters_callback);
            return text;
        }

        var charInsideUrl = "[-A-Z0-9+&@#/%?=~_|[\\]()!:,.;]",
            charEndingUrl = "[-A-Z0-9+&@#/%=~_|[\\])]",
            autoLinkRegex = new RegExp("(=\"|<)?\\b(https?|ftp)(://" + charInsideUrl + "*" + charEndingUrl + ")(?=$|\\W)", "gi"),
            endCharRegex = new RegExp(charEndingUrl, "i");

        function handleTrailingParens(wholeMatch, lookbehind, protocol, link, index, str) {

            if (/^<[^<>]*(https?|ftp)/.test(str)) {
                //避免 html 标签内 属性值的 超链接被替换为 a 标签（例如 img 的src 属性）
                return wholeMatch;
            }
            if (lookbehind)
                return wholeMatch;
            if (link.charAt(link.length - 1) !== ")")
                return "<" + protocol + link + ">";
            var parens = link.match(/[()]/g);
            var level = 0;
            for (var i = 0; i < parens.length; i++) {
                if (parens[i] === "(") {
                    if (level <= 0)
                        level = 1;
                    else
                        level++;
                }
                else {
                    level--;
                }
            }
            var tail = "";
            if (level < 0) {
                var re = new RegExp("\\){1," + (-level) + "}$");
                link = link.replace(re, function (trailingParens) {
                    tail = trailingParens;
                    return "";
                });
            }
            if (tail) {
                var lastChar = link.charAt(link.length - 1);
                if (!endCharRegex.test(lastChar)) {
                    tail = lastChar + tail;
                    link = link.substr(0, link.length - 1);
                }
            }
            return "<" + protocol + link + ">" + tail;
        }

        function _DoAutoLinks(text) {

            // note that at this point, all other URL in the text are already hyperlinked as <a href=""></a>
            // *except* for the <http://www.foo.com> case

            // automatically add < and > around unadorned raw hyperlinks
            // must be preceded by a non-word character (and not by =" or <) and followed by non-word/EOF character
            // simulating the lookbehind in a consuming way is okay here, since a URL can neither and with a " nor
            // with a <, so there is no risk of overlapping matches.
            text = text.replace(autoLinkRegex, handleTrailingParens);

            //  autolink anything like <http://example.com>


            var replacer = function (wholematch, m1) {
                var url = attributeSafeUrl(m1);

                return "<a href=\"" + url + "\">" + pluginHooks.plainLinkText(m1) + "</a>";
            };
            text = text.replace(/<((https?|ftp):[^'">\s]+)>/gi, replacer);

            // Email addresses: <address@domain.foo>
            /*
             text = text.replace(/
             <
             (?:mailto:)?
             (
             [-.\w]+
             \@
             [-a-z0-9]+(\.[-a-z0-9]+)*\.[a-z]+
             )
             >
             /gi, _DoAutoLinks_callback());
             */

            /* disabling email autolinking, since we don't do that on the server, either
             text = text.replace(/<(?:mailto:)?([-.\w]+\@[-a-z0-9]+(\.[-a-z0-9]+)*\.[a-z]+)>/gi,
             function(wholeMatch,m1) {
             return _EncodeEmailAddress( _UnescapeSpecialChars(m1) );
             }
             );
             */
            return text;
        }

        function _UnescapeSpecialChars(text) {
            //
            // Swap back in all the special characters we've hidden.
            //
            text = text.replace(/~E(\d+)E/g,
                function (wholeMatch, m1) {
                    var charCodeToReplace = parseInt(m1);
                    return String.fromCharCode(charCodeToReplace);
                }
            );
            return text;
        }

        function _Outdent(text) {
            //
            // Remove one level of line-leading tabs or spaces
            //

            // attacklab: hack around Konqueror 3.5.4 bug:
            // "----------bug".replace(/^-/g,"") == "bug"

            text = text.replace(/^(\t|[ ]{1,4})/gm, "~0"); // attacklab: g_tab_width

            // attacklab: clean up hack
            text = text.replace(/~0/g, "")

            return text;
        }

        function _Detab(text) {
            if (!/\t/.test(text))
                return text;

            var spaces = ["    ", "   ", "  ", " "],
                skew = 0,
                v;

            return text.replace(/[\n\t]/g, function (match, offset) {
                if (match === "\n") {
                    skew = offset + 1;
                    return match;
                }
                v = (offset - skew) % 4;
                skew = offset + 1;
                return spaces[v];
            });
        }

        //
        //  attacklab: Utility functions
        //

        function attributeSafeUrl(url) {
            url = attributeEncode(url);
            url = escapeCharacters(url, "*_:()[]")
            return url;
        }

        function escapeCharacters(text, charsToEscape, afterBackslash) {
            // First we have to escape the escape characters so that
            // we can build a character class out of them
            var regexString = "([" + charsToEscape.replace(/([\[\]\\])/g, "\\$1") + "])";

            if (afterBackslash) {
                regexString = "\\\\" + regexString;
            }

            var regex = new RegExp(regexString, "g");
            text = text.replace(regex, escapeCharacters_callback);

            return text;
        }


        function escapeCharacters_callback(wholeMatch, m1) {
            var charCodeToEscape = m1.charCodeAt(0);
            return "~E" + charCodeToEscape + "E";
        }

    }; // end of the Markdown.Converter constructor

})();

module.exports = Markdown;
},{}],45:[function(require,module,exports){
var Markdown = {};

(function () {
    // A quick way to make sure we're only keeping span-level tags when we need to.
    // This isn't supposed to be foolproof. It's just a quick way to make sure we
    // keep all span-level tags returned by a pagedown converter. It should allow
    // all span-level tags through, with or without attributes.
    var inlineTags = new RegExp(['^(<\\/?(a|abbr|acronym|applet|area|b|basefont|',
        'bdo|big|button|cite|code|del|dfn|em|figcaption|',
        'font|i|iframe|img|input|ins|kbd|label|map|',
        'mark|meter|object|param|progress|q|ruby|rp|rt|s|',
        'samp|script|select|small|span|strike|strong|',
        'sub|sup|textarea|time|tt|u|var|wbr)[^>]*>|',
        '<(br)\\s?\\/?>)$'].join(''), 'i');

    /******************************************************************
     * Utility Functions                                              *
     *****************************************************************/

    // patch for ie7
    if (!Array.indexOf) {
        Array.prototype.indexOf = function (obj) {
            for (var i = 0; i < this.length; i++) {
                if (this[i] == obj) {
                    return i;
                }
            }
            return -1;
        };
    }

    function trim(str) {
        return str.replace(/^\s+|\s+$/g, '');
    }

    function rtrim(str) {
        return str.replace(/\s+$/g, '');
    }

    // Remove one level of indentation from text. Indent is 4 spaces.
    function outdent(text) {
        return text.replace(new RegExp('^(\\t|[ ]{1,4})', 'gm'), '');
    }

    function contains(str, substr) {
        return str.indexOf(substr) != -1;
    }

    // Sanitize html, removing tags that aren't in the whitelist
    function sanitizeHtml(html, whitelist) {
        return html.replace(/<[^>]*>?/gi, function (tag) {
            return tag.match(whitelist) ? tag : '';
        });
    }

    // Merge two arrays, keeping only unique elements.
    function union(x, y) {
        var obj = {};
        for (var i = 0; i < x.length; i++)
            obj[x[i]] = x[i];
        for (i = 0; i < y.length; i++)
            obj[y[i]] = y[i];
        var res = [];
        for (var k in obj) {
            if (obj.hasOwnProperty(k))
                res.push(obj[k]);
        }
        return res;
    }

    // JS regexes don't support \A or \Z, so we add sentinels, as Pagedown
    // does. In this case, we add the ascii codes for start of text (STX) and
    // end of text (ETX), an idea borrowed from:
    // https://github.com/tanakahisateru/js-markdown-extra
    function addAnchors(text) {
        if (text.charAt(0) != '\x02')
            text = '\x02' + text;
        if (text.charAt(text.length - 1) != '\x03')
            text = text + '\x03';
        return text;
    }

    // Remove STX and ETX sentinels.
    function removeAnchors(text) {
        if (text.charAt(0) == '\x02')
            text = text.substr(1);
        if (text.charAt(text.length - 1) == '\x03')
            text = text.substr(0, text.length - 1);
        return text;
    }

    // Convert markdown within an element, retaining only span-level tags
    function convertSpans(text, extra) {
        return sanitizeHtml(convertAll(text, extra), inlineTags);
    }

    // Convert internal markdown using the stock pagedown converter
    function convertAll(text, extra) {
        var result = extra.blockGamutHookCallback(text);
        // We need to perform these operations since we skip the steps in the converter
        result = unescapeSpecialChars(result);
        result = result.replace(/~D/g, "$$").replace(/~T/g, "~");
        result = extra.previousPostConversion(result);
        return result;
    }

    // Convert escaped special characters
    function processEscapesStep1(text) {
        // Markdown extra adds two escapable characters, `:` and `|`
        return text.replace(/\\\|/g, '~I').replace(/\\:/g, '~i');
    }

    function processEscapesStep2(text) {
        return text.replace(/~I/g, '|').replace(/~i/g, ':');
    }

    // Duplicated from PageDown converter
    function unescapeSpecialChars(text) {
        // Swap back in all the special characters we've hidden.
        text = text.replace(/~E(\d+)E/g, function (wholeMatch, m1) {
            var charCodeToReplace = parseInt(m1);
            return String.fromCharCode(charCodeToReplace);
        });
        return text;
    }

    function slugify(text) {
        return text.toLowerCase()
            .replace(/\s+/g, '-') // Replace spaces with -
            .replace(/[^\w\-]+/g, '') // Remove all non-word chars
            .replace(/\-\-+/g, '-') // Replace multiple - with single -
            .replace(/^-+/, '') // Trim - from start of text
            .replace(/-+$/, ''); // Trim - from end of text
    }

    /*****************************************************************************
     * Markdown.Extra *
     ****************************************************************************/

    Markdown.Extra = function () {
        // For converting internal markdown (in tables for instance).
        // This is necessary since these methods are meant to be called as
        // preConversion hooks, and the Markdown converter passed to init()
        // won't convert any markdown contained in the html tags we return.
        this.converter = null;

        // Stores html blocks we generate in hooks so that
        // they're not destroyed if the user is using a sanitizing converter
        this.hashBlocks = [];

        // Stores footnotes
        this.footnotes = {};
        this.usedFootnotes = [];

        // Special attribute blocks for fenced code blocks and headers enabled.
        this.attributeBlocks = false;

        // Fenced code block options
        this.googleCodePrettify = false;
        this.highlightJs = false;
        this.codeMirror = false;

        // Table options
        this.tableClass = '';

        this.tabWidth = 4;
    };

    Markdown.Extra.init = function (converter, options) {
        // Each call to init creates a new instance of Markdown.Extra so it's
        // safe to have multiple converters, with different options, on a single page
        var extra = new Markdown.Extra();
        var postNormalizationTransformations = [];
        var preBlockGamutTransformations = [];
        var postSpanGamutTransformations = [];
        var postConversionTransformations = ["unHashExtraBlocks"];

        options = options || {};
        options.extensions = options.extensions || ["all"];
        if (contains(options.extensions, "all")) {
            options.extensions = ["tables", "fenced_code_gfm", "def_list", "attr_list", "footnotes", "smartypants",
                "todo_list", "strikethrough", "newlines"];
        }
        preBlockGamutTransformations.push("wrapHeaders");

        if (contains(options.extensions, "todo_list")) {
            preBlockGamutTransformations.push("todo_list");
        }
        if (contains(options.extensions, "attr_list")) {
            postNormalizationTransformations.push("hashFcbAttributeBlocks");
            preBlockGamutTransformations.push("hashHeaderAttributeBlocks");
            postConversionTransformations.push("applyAttributeBlocks");
            extra.attributeBlocks = true;
        }
        if (contains(options.extensions, "fenced_code_gfm")) {
            // This step will convert fcb inside list items and blockquotes
            preBlockGamutTransformations.push("fencedCodeBlocks");
            // This extra step is to prevent html blocks hashing and link definition/footnotes stripping inside fcb
            postNormalizationTransformations.push("fencedCodeBlocks");
        }
        if (contains(options.extensions, "tables")) {
            preBlockGamutTransformations.push("tables");
        }
        if (contains(options.extensions, "def_list")) {
            preBlockGamutTransformations.push("definitionLists");
        }
        if (contains(options.extensions, "footnotes")) {
            postNormalizationTransformations.push("stripFootnoteDefinitions");
            preBlockGamutTransformations.push("doFootnotes");
            postConversionTransformations.push("printFootnotes");
        }
        if (contains(options.extensions, "smartypants")) {
            postConversionTransformations.push("runSmartyPants");
        }
        if (contains(options.extensions, "strikethrough")) {
            postSpanGamutTransformations.push("strikethrough");
        }
        if (contains(options.extensions, "newlines")) {
            postSpanGamutTransformations.push("newlines");
        }

        converter.hooks.chain("postNormalization", function (text) {
            return extra.doTransform(postNormalizationTransformations, text) + '\n';
        });

        converter.hooks.chain("preBlockGamut", function (text, blockGamutHookCallback) {
            // Keep a reference to the block gamut callback to run recursively
            extra.blockGamutHookCallback = blockGamutHookCallback;
            text = processEscapesStep1(text);
            text = extra.doTransform(preBlockGamutTransformations, text) + '\n';
            text = processEscapesStep2(text);
            return text;
        });

        converter.hooks.chain("postSpanGamut", function (text) {
            return extra.doTransform(postSpanGamutTransformations, text);
        });

        // Keep a reference to the hook chain running before doPostConversion to apply on hashed extra blocks
        extra.previousPostConversion = converter.hooks.postConversion;
        converter.hooks.chain("postConversion", function (text) {
            text = extra.doTransform(postConversionTransformations, text);
            // Clear state vars that may use unnecessary memory
            extra.hashBlocks = [];
            extra.footnotes = {};
            extra.usedFootnotes = [];
            return text;
        });

        if ("highlighter" in options) {
            extra.googleCodePrettify = options.highlighter === 'prettify';
            extra.highlightJs = options.highlighter === 'highlight';
            extra.codeMirror = options.highlighter === 'codeMirror';
        }

        if ("table_class" in options) {
            extra.tableClass = options.table_class;
        }

        extra.converter = converter;

        // Caller usually won't need this, but it's handy for testing.
        return extra;
    };

    // Do transformations
    Markdown.Extra.prototype.doTransform = function (transformations, text) {
        for (var i = 0; i < transformations.length; i++)
            text = this[transformations[i]](text);
        return text;
    };

    // Return a placeholder containing a key, which is the block's index in the
    // hashBlocks array. We wrap our output in a <p> tag here so Pagedown won't.
    Markdown.Extra.prototype.hashExtraBlock = function (block) {
        return '\n<p>~X' + (this.hashBlocks.push(block) - 1) + 'X</p>\n';
    };
    Markdown.Extra.prototype.hashExtraInline = function (block) {
        return '~X' + (this.hashBlocks.push(block) - 1) + 'X';
    };

    // Replace placeholder blocks in `text` with their corresponding
    // html blocks in the hashBlocks array.
    Markdown.Extra.prototype.unHashExtraBlocks = function (text) {
        var self = this;

        function recursiveUnHash() {
            var hasHash = false;
            text = text.replace(/(?:<p>)?~X(\d+)X(?:<\/p>)?/g, function (wholeMatch, m1) {
                hasHash = true;
                var key = parseInt(m1, 10);
                return self.hashBlocks[key];
            });
            if (hasHash === true) {
                recursiveUnHash();
            }
        }

        recursiveUnHash();
        return text;
    };

    // Wrap headers to make sure they won't be in def lists
    Markdown.Extra.prototype.wrapHeaders = function (text) {
        function wrap(text) {
            return '\n' + text + '\n';
        }

        text = text.replace(/^.+[ \t]*\n=+[ \t]*\n+/gm, wrap);
        text = text.replace(/^.+[ \t]*\n-+[ \t]*\n+/gm, wrap);
        text = text.replace(/^\#{1,6}[ \t]*.+?[ \t]*\#*\n+/gm, wrap);
        return text;
    };


    /******************************************************************
     * Attribute Blocks                                               *
     *****************************************************************/

    // TODO: use sentinels. Should we just add/remove them in doConversion?
    // TODO: better matches for id / class attributes
    var attrBlock = "\\{[ \\t]*((?:[#.][-_:a-zA-Z0-9]+[ \\t]*)+)\\}";
    var hdrAttributesA = new RegExp("^(#{1,6}.*#{0,6})[ \\t]+" + attrBlock + "[ \\t]*(?:\\n|0x03)", "gm");
    var hdrAttributesB = new RegExp("^(.*)[ \\t]+" + attrBlock + "[ \\t]*\\n" +
        "(?=[\\-|=]+\\s*(?:\\n|0x03))", "gm"); // underline lookahead
    var fcbAttributes = new RegExp("^(```[ \\t]*[^{\\s]*)[ \\t]+" + attrBlock + "[ \\t]*\\n" +
        "(?=([\\s\\S]*?)\\n```[ \\t]*(\\n|0x03))", "gm");

    // Extract headers attribute blocks, move them above the element they will be
    // applied to, and hash them for later.
    Markdown.Extra.prototype.hashHeaderAttributeBlocks = function (text) {

        var self = this;

        function attributeCallback(wholeMatch, pre, attr) {
            return '<p>~XX' + (self.hashBlocks.push(attr) - 1) + 'XX</p>\n' + pre + "\n";
        }

        text = text.replace(hdrAttributesA, attributeCallback);  // ## headers
        text = text.replace(hdrAttributesB, attributeCallback);  // underline headers
        return text;
    };

    // Extract FCB attribute blocks, move them above the element they will be
    // applied to, and hash them for later.
    Markdown.Extra.prototype.hashFcbAttributeBlocks = function (text) {
        // TODO: use sentinels. Should we just add/remove them in doConversion?
        // TODO: better matches for id / class attributes

        var self = this;

        function attributeCallback(wholeMatch, pre, attr) {
            return '<p>~XX' + (self.hashBlocks.push(attr) - 1) + 'XX</p>\n' + pre + "\n";
        }

        return text.replace(fcbAttributes, attributeCallback);
    };

    Markdown.Extra.prototype.applyAttributeBlocks = function (text) {
        var self = this;
        var blockRe = new RegExp('<p>~XX(\\d+)XX</p>[\\s]*' +
            '(?:<(h[1-6]|pre)(?: +class="(\\S+)")?(>[\\s\\S]*?</\\2>))', "gm");
        text = text.replace(blockRe, function (wholeMatch, k, tag, cls, rest) {
            if (!tag) // no following header or fenced code block.
                return '';

            // get attributes list from hash
            var key = parseInt(k, 10);
            var attributes = self.hashBlocks[key];

            // get id
            var id = attributes.match(/#[^\s#.]+/g) || [];
            var idStr = id[0] ? ' id="' + id[0].substr(1, id[0].length - 1) + '"' : '';

            // get classes and merge with existing classes
            var classes = attributes.match(/\.[^\s#.]+/g) || [];
            for (var i = 0; i < classes.length; i++) // Remove leading dot
                classes[i] = classes[i].substr(1, classes[i].length - 1);

            var classStr = '';
            if (cls)
                classes = union(classes, [cls]);

            if (classes.length > 0)
                classStr = ' class="' + classes.join(' ') + '"';

            return "<" + tag + idStr + classStr + rest;
        });

        return text;
    };

    /******************************************************************
     * Tables                                                         *
     *****************************************************************/

    // Find and convert Markdown Extra tables into html.
    Markdown.Extra.prototype.tables = function (text) {
        var self = this;

        var leadingPipe = new RegExp(
            ['^',
                '[ ]{0,3}', // Allowed whitespace
                '[|]', // Initial pipe
                '(.+)\\n', // $1: Header Row

                '[ ]{0,3}', // Allowed whitespace
                '[|]([ ]*[-:]+[-| :]*)\\n', // $2: Separator

                '(', // $3: Table Body
                '(?:[ ]*[|].*\\n?)*', // Table rows
                ')',
                '(?:\\n|$)'                   // Stop at final newline
            ].join(''),
            'gm'
        );

        var noLeadingPipe = new RegExp(
            ['^',
                '[ ]{0,3}', // Allowed whitespace
                '(\\S.*[|].*)\\n', // $1: Header Row

                '[ ]{0,3}', // Allowed whitespace
                '([-:]+[ ]*[|][-| :]*)\\n', // $2: Separator

                '(', // $3: Table Body
                '(?:.*[|].*\\n?)*', // Table rows
                ')',
                '(?:\\n|$)'                   // Stop at final newline
            ].join(''),
            'gm'
        );

        text = text.replace(leadingPipe, doTable);
        text = text.replace(noLeadingPipe, doTable);

        // $1 = header, $2 = separator, $3 = body
        function doTable(match, header, separator, body, offset, string) {
            // remove any leading pipes and whitespace
            header = header.replace(/^ *[|]/m, '');
            separator = separator.replace(/^ *[|]/m, '');
            body = body.replace(/^ *[|]/gm, '');

            // remove trailing pipes and whitespace
            header = header.replace(/[|] *$/m, '');
            separator = separator.replace(/[|] *$/m, '');
            body = body.replace(/[|] *$/gm, '');

            // determine column alignments
            var alignspecs = separator.split(/ *[|] */);
            var align = [];
            for (var i = 0; i < alignspecs.length; i++) {
                var spec = alignspecs[i];
                if (spec.match(/^ *-+: *$/m))
                    align[i] = ' align="right"';
                else if (spec.match(/^ *:-+: *$/m))
                    align[i] = ' align="center"';
                else if (spec.match(/^ *:-+ *$/m))
                    align[i] = ' align="left"';
                else align[i] = '';
            }

            // TODO: parse spans in header and rows before splitting, so that pipes
            // inside of tags are not interpreted as separators
            var headers = header.split(/ *[|] */);
            var colCount = headers.length;

            // build html
            var cls = self.tableClass ? ' class="' + self.tableClass + '"' : '';
            var html = ['<table', cls, '>\n', '<thead>\n', '<tr>\n'].join('');

            // build column headers.
            for (i = 0; i < colCount; i++) {
                var headerHtml = convertSpans(trim(headers[i]), self);
                html += ["  <th", align[i], ">", headerHtml, "</th>\n"].join('');
            }
            html += "</tr>\n</thead>\n";

            // build rows
            var rows = body.split('\n');
            for (i = 0; i < rows.length; i++) {
                if (rows[i].match(/^\s*$/)) // can apply to final row
                    continue;

                // ensure number of rowCells matches colCount
                var rowCells = rows[i].split(/ *[|] */);
                var lenDiff = colCount - rowCells.length;
                for (var j = 0; j < lenDiff; j++)
                    rowCells.push('');

                html += "<tr>\n";
                for (j = 0; j < colCount; j++) {
                    var colHtml = convertSpans(trim(rowCells[j]), self);
                    html += ["  <td", align[j], ">", colHtml, "</td>\n"].join('');
                }
                html += "</tr>\n";
            }

            html += "</table>\n";

            // replace html with placeholder until postConversion step
            return self.hashExtraBlock(html);
        }

        return text;
    };


    /******************************************************************
     * Footnotes                                                      *
     *****************************************************************/

    // Strip footnote, store in hashes.
    Markdown.Extra.prototype.stripFootnoteDefinitions = function (text) {
        var self = this;

        text = text.replace(
            /\n[ ]{0,3}\[\^(.+?)\]\:[ \t]*\n?([\s\S]*?)\n{1,2}((?=\n[ ]{0,3}\S)|$)/g,
            function (wholeMatch, m1, m2) {
                m1 = slugify(m1);
                m2 += "\n";
                m2 = m2.replace(/^[ ]{0,3}/g, "");
                self.footnotes[m1] = m2;
                return "\n";
            });

        return text;
    };


    // Find and convert footnotes references.
    Markdown.Extra.prototype.doFootnotes = function (text) {
        var self = this;
        if (self.isConvertingFootnote === true) {
            return text;
        }

        var footnoteCounter = 0;
        text = text.replace(/\[\^(.+?)\]/g, function (wholeMatch, m1) {
            var id = slugify(m1);
            var footnote = self.footnotes[id];
            if (footnote === undefined) {
                return wholeMatch;
            }
            footnoteCounter++;
            self.usedFootnotes.push(id);
            var html = '<a href="#fn_' + id + '" id="fnref_' + id
                + '" title="See footnote" class="footnote">' + footnoteCounter
                + '</a>';
            return self.hashExtraInline(html);
        });

        return text;
    };

    // Print footnotes at the end of the document
    Markdown.Extra.prototype.printFootnotes = function (text) {
        var self = this;

        if (self.usedFootnotes.length === 0) {
            return text;
        }

        text += '\n\n<div class="footnotes">\n<hr>\n<ol>\n\n';
        for (var i = 0; i < self.usedFootnotes.length; i++) {
            var id = self.usedFootnotes[i];
            var footnote = self.footnotes[id];
            self.isConvertingFootnote = true;
            var formattedfootnote = convertSpans(footnote, self);
            delete self.isConvertingFootnote;
            text += '<li id="fn_'
                + id
                + '">'
                + formattedfootnote
                + ' <a href="#fnref_'
                + id
                + '" title="Return to article" class="reversefootnote">&#8617;</a></li>\n\n';
        }
        text += '</ol>\n</div>';
        return text;
    };


    /******************************************************************
     * Fenced Code Blocks  (gfm)                                       *
     ******************************************************************/

    // Find and convert gfm-inspired fenced code blocks into html.
    Markdown.Extra.prototype.fencedCodeBlocks = function (text) {
        function encodeCode(code) {
            code = code.replace(/&/g, "&amp;");
            code = code.replace(/</g, "&lt;");
            code = code.replace(/>/g, "&gt;");
            // These were escaped by PageDown before postNormalization
            code = code.replace(/~D/g, "$$");
            code = code.replace(/~T/g, "~");
            return code;
        }

        var self = this;
        text = text.replace(/(?:^|\n)```[ \t]*(\S*)[ \t]*\n([\s\S]*?)\n```[ \t]*(?=\n)/g, function (match, m1, m2) {
            var codeOption = m1.replace(';', ',').split(','), codeblock = m2,
                language = codeOption[0].trim(), theme;
            if (codeOption.length > 1) {
                theme = codeOption[1].trim();
            }

            var preclass, codeclass, html;
            // 流程图、序列图保持原样
            if (language === 'flow' || language === 'sequence') {
                codeclass = ' class="language-' + language + '"';
                html = ['<pre><textarea readonly style="display:none;"', codeclass, '>',
                    encodeCode(codeblock), '</textarea></pre>'].join('');
            } else if (self.codeMirror) {
                html = ['<div class="wiz-code-container" contenteditable="false"',
                    ' data-mode="', language, '" data-theme="', theme, '"><textarea readonly style="display:none;">',
                    encodeCode(codeblock), '</textarea></div>'].join('');

            } else {
                // adhere to specified options
                preclass = self.googleCodePrettify ? ' class="prettyprint linenums"' : '';
                codeclass = '';
                if (language) {
                    if (self.codeMirror || self.googleCodePrettify || self.highlightJs) {
                        // use html5 language- class names. supported by both prettify and highlight.js
                        codeclass = ' class="language-' + language + '"';
                    } else {
                        codeclass = ' class="' + language + '"';
                    }
                }

                html = ['<pre', preclass, '><code', codeclass, '>',
                    encodeCode(codeblock), '</code></pre>'].join('');
            }

            // replace codeblock with placeholder until postConversion step
            return self.hashExtraBlock(html);
        });

        return text;
    };


    /******************************************************************
     * SmartyPants                                                     *
     ******************************************************************/

    Markdown.Extra.prototype.educatePants = function (text) {
        var self = this;
        var result = '';
        var blockOffset = 0;
        // Here we parse HTML in a very bad manner
        text.replace(/(?:<!--[\s\S]*?-->)|(<)([a-zA-Z1-6]+)([^\n]*?>)([\s\S]*?)(<\/\2>)/g, function (wholeMatch, m1, m2, m3, m4, m5, offset) {
            var token = text.substring(blockOffset, offset);
            result += self.applyPants(token);
            self.smartyPantsLastChar = result.substring(result.length - 1);
            blockOffset = offset + wholeMatch.length;
            if (!m1) {
                // Skip commentary
                result += wholeMatch;
                return;
            }
            // Skip special tags
            if (!/code|kbd|pre|script|noscript|iframe|math|ins|del|pre/i.test(m2)) {
                m4 = self.educatePants(m4);
            }
            else {
                self.smartyPantsLastChar = m4.substring(m4.length - 1);
            }
            result += m1 + m2 + m3 + m4 + m5;
        });
        var lastToken = text.substring(blockOffset);
        result += self.applyPants(lastToken);
        self.smartyPantsLastChar = result.substring(result.length - 1);
        return result;
    };

    function revertPants(wholeMatch, m1) {
        var blockText = m1;
        blockText = blockText.replace(/&\#8220;/g, "\"");
        blockText = blockText.replace(/&\#8221;/g, "\"");
        blockText = blockText.replace(/&\#8216;/g, "'");
        blockText = blockText.replace(/&\#8217;/g, "'");
        blockText = blockText.replace(/&\#8212;/g, "---");
        blockText = blockText.replace(/&\#8211;/g, "--");
        blockText = blockText.replace(/&\#8230;/g, "...");
        return blockText;
    }

    Markdown.Extra.prototype.applyPants = function (text) {
        // Dashes
        // text = text.replace(/---/g, "&#8212;").replace(/--/g, "&#8211;");
        // Ellipses
        text = text.replace(/\.\.\./g, "&#8230;").replace(/\.\s\.\s\./g, "&#8230;");
        // Backticks
        // text = text.replace(/``/g, "&#8220;").replace(/''/g, "&#8221;");

        // if (/^'$/.test(text)) {
        //   // Special case: single-character ' token
        //   if (/\S/.test(this.smartyPantsLastChar)) {
        //     return "&#8217;";
        //   }
        //   return "&#8216;";
        // }
        // if (/^"$/.test(text)) {
        //   // Special case: single-character " token
        //   if (/\S/.test(this.smartyPantsLastChar)) {
        //     return "&#8221;";
        //   }
        //   return "&#8220;";
        // }

        // Special case if the very first character is a quote
        // followed by punctuation at a non-word-break. Close the quotes by brute force:
        // text = text.replace(/^'(?=[!"#\$\%'()*+,\-.\/:;<=>?\@\[\\]\^_`{|}~]\B)/, "&#8217;");
        // text = text.replace(/^"(?=[!"#\$\%'()*+,\-.\/:;<=>?\@\[\\]\^_`{|}~]\B)/, "&#8221;");

        // Special case for double sets of quotes, e.g.:
        //   <p>He said, "'Quoted' words in a larger quote."</p>
        // text = text.replace(/"'(?=\w)/g, "&#8220;&#8216;");
        // text = text.replace(/'"(?=\w)/g, "&#8216;&#8220;");

        // Special case for decade abbreviations (the '80s):
        // text = text.replace(/'(?=\d{2}s)/g, "&#8217;");

        // Get most opening single quotes:
        // text = text.replace(/(\s|&nbsp;|--|&[mn]dash;|&\#8211;|&\#8212;|&\#x201[34];)'(?=\w)/g, "$1&#8216;");

        // Single closing quotes:
        // text = text.replace(/([^\s\[\{\(\-])'/g, "$1&#8217;");
        // text = text.replace(/'(?=\s|s\b)/g, "&#8217;");

        // Any remaining single quotes should be opening ones:
        // text = text.replace(/'/g, "&#8216;");

        // Get most opening double quotes:
        // text = text.replace(/(\s|&nbsp;|--|&[mn]dash;|&\#8211;|&\#8212;|&\#x201[34];)"(?=\w)/g, "$1&#8220;");

        // Double closing quotes:
        // text = text.replace(/([^\s\[\{\(\-])"/g, "$1&#8221;");
        // text = text.replace(/"(?=\s)/g, "&#8221;");

        // Any remaining quotes should be opening ones.
        // text = text.replace(/"/ig, "&#8220;");
        return text;
    };

    // Find and convert markdown extra definition lists into html.
    Markdown.Extra.prototype.runSmartyPants = function (text) {
        this.smartyPantsLastChar = '';
        text = this.educatePants(text);
        // Clean everything inside html tags (some of them may have been converted due to our rough html parsing)
        text = text.replace(/(<([a-zA-Z1-6]+)\b([^\n>]*?)(\/)?>)/g, revertPants);
        return text;
    };

    /******************************************************************
     * Definition Lists                                                *
     ******************************************************************/

    // Find and convert markdown extra definition lists into html.
    Markdown.Extra.prototype.definitionLists = function (text) {
        var wholeList = new RegExp(
            ['(\\x02\\n?|\\n\\n)',
                '(?:',
                '(', // $1 = whole list
                '(', // $2
                '[ ]{0,3}',
                '((?:[ \\t]*\\S.*\\n)+)', // $3 = defined term
                '\\n?',
                '[ ]{0,3}:[ ]+', // colon starting definition
                ')',
                '([\\s\\S]+?)',
                '(', // $4
                '(?=\\0x03)', // \z
                '|',
                '(?=',
                '\\n{2,}',
                '(?=\\S)',
                '(?!', // Negative lookahead for another term
                '[ ]{0,3}',
                '(?:\\S.*\\n)+?', // defined term
                '\\n?',
                '[ ]{0,3}:[ ]+', // colon starting definition
                ')',
                '(?!', // Negative lookahead for another definition
                '[ ]{0,3}:[ ]+', // colon starting definition
                ')',
                ')',
                ')',
                ')',
                ')'
            ].join(''),
            'gm'
        );

        var self = this;
        text = addAnchors(text);

        text = text.replace(wholeList, function (match, pre, list) {
            var result = trim(self.processDefListItems(list));
            result = "<dl>\n" + result + "\n</dl>";
            return pre + self.hashExtraBlock(result) + "\n\n";
        });

        return removeAnchors(text);
    };

    // Process the contents of a single definition list, splitting it
    // into individual term and definition list items.
    Markdown.Extra.prototype.processDefListItems = function (listStr) {
        var self = this;

        var dt = new RegExp(
            ['(\\x02\\n?|\\n\\n+)', // leading line
                '(', // definition terms = $1
                '[ ]{0,3}', // leading whitespace
                '(?![:][ ]|[ ])', // negative lookahead for a definition
                                  //   mark (colon) or more whitespace
                '(?:\\S.*\\n)+?', // actual term (not whitespace)
                ')',
                '(?=\\n?[ ]{0,3}:[ ])'     // lookahead for following line feed
            ].join(''),                 //   with a definition mark
            'gm'
        );

        var dd = new RegExp(
            ['\\n(\\n+)?', // leading line = $1
                '(', // marker space = $2
                '[ ]{0,3}', // whitespace before colon
                '[:][ ]+', // definition mark (colon)
                ')',
                '([\\s\\S]+?)', // definition text = $3
                '(?=\\n*', // stop at next definition mark,
                '(?:', // next term or end of text
                '\\n[ ]{0,3}[:][ ]|',
                '<dt>|\\x03', // \z
                ')',
                ')'
            ].join(''),
            'gm'
        );

        listStr = addAnchors(listStr);
        // trim trailing blank lines:
        listStr = listStr.replace(/\n{2,}(?=\\x03)/, "\n");

        // Process definition terms.
        listStr = listStr.replace(dt, function (match, pre, termsStr) {
            var terms = trim(termsStr).split("\n");
            var text = '';
            for (var i = 0; i < terms.length; i++) {
                var term = terms[i];
                // process spans inside dt
                term = convertSpans(trim(term), self);
                text += "\n<dt>" + term + "</dt>";
            }
            return text + "\n";
        });

        // Process actual definitions.
        listStr = listStr.replace(dd, function (match, leadingLine, markerSpace, def) {
            if (leadingLine || def.match(/\n{2,}/)) {
                // replace marker with the appropriate whitespace indentation
                def = Array(markerSpace.length + 1).join(' ') + def;
                // process markdown inside definition
                // TODO?: currently doesn't apply extensions
                def = outdent(def) + "\n\n";
                def = "\n" + convertAll(def, self) + "\n";
            } else {
                // convert span-level markdown inside definition
                def = rtrim(def);
                def = convertSpans(outdent(def), self);
            }

            return "\n<dd>" + def + "</dd>\n";
        });

        return removeAnchors(listStr);
    };

    /***********************************************************
     * todoList                                            *
     ************************************************************/
    Markdown.Extra.prototype.todo_list = function (text) {
        return text.replace(/^([ ]*)\[ \]/g, "$1<input type='checkbox' disabled class='wiz-md-todo-checkbox'>")
            .replace(/^([ ]*)\[x\]/g, "$1<input type='checkbox' disabled checked class='wiz-md-todo-checkbox'>");
    };

    /***********************************************************
     * Strikethrough                                            *
     ************************************************************/

    Markdown.Extra.prototype.strikethrough = function (text) {
        // Pretty much duplicated from _DoItalicsAndBold
        // return text.replace(/([\W_]|^)~T~T(?=\S)([^\r]*?\S[\*_]*)~T~T([\W_]|$)/gm,
        //     "$1<del>$2</del>$3");
        return text.replace(/~T~T(?=\S)([^\r]*?\S[\*_]*)~T~T/g,
            "<del>$1</del>");
    };


    /***********************************************************
     * New lines                                                *
     ************************************************************/

    Markdown.Extra.prototype.newlines = function (text) {
        // We have to ignore already converted newlines and line breaks in sub-list items
        return text.replace(/(<(?:br|\/li)>)?\n/g, function (wholeMatch, previousTag) {
            return previousTag ? wholeMatch : " <br>\n";
        });
    };

})();

module.exports  = Markdown.Extra;

},{}],46:[function(require,module,exports){
/**
 * markdown & mathjax 渲染处理
 */

var ENV = require('../common/env'),
    utils = require('../common/utils'),
    dependLoader = require('../common/dependLoader'),
    scriptLoader = require('../common/scriptLoader'),
    domUtils = require('../domUtils/domBase'),
    xssUtils = require('../domUtils/xssUtils'),
    markdownBase = require('./Markdown.Converter'),
    markdownExtra = require('./Markdown.Extra');

var isMathjax = false;
var WizToc = '#wizToc';

var defalutCB = {
    markdown: function () {
        Render.Win.prettyPrint();
        Render.tocRender();
        Render.flowRender();
        Render.sequenceRender();
    },
    mathJax: function () {

    }
};

/**
 * 修正 safari innerText 的 bug
 * （math、semantics、annotation 等 tag 输出 innerText 会出现换行，而且 <math>...</math> 内的内容会被删除）
 */
function patchForSafari () {
    var tmp = Render.Document.createElement('span');
    tmp.innerHTML = '<math>a</math>';
    tmp.style.opacity = 0;
    Render.Document.body.appendChild(tmp);
    var tmpText = tmp.innerText;
    domUtils.remove(tmp);
    if (tmpText) {
        return;
    }

    var tagList = Render.Document.getElementsByTagName('math');
    var tag, parent, span;
    while (tagList.length > 0) {
        tag = tagList[0];
        parent = tag.parentNode;
        span = Render.Document.createElement('span');
        span.innerText = tag.textContent;
        parent.insertBefore(span, tag);
        parent.removeChild(tag);
    }
}

var MarkdownRender = {
    init: function () {
        Render.Win = ENV.win;
        Render.Document = ENV.doc;
        Render.Dependency = ENV.dependency;

        return MarkdownRender;
    },
    markdown: function (callback) {
        if (callback) {
            Render.callback.markdown = Render.addCb(defalutCB.markdown, callback.markdown);
            Render.callback.mathJax = Render.addCb(defalutCB.mathJax, callback.mathJax);
        }

        dependLoader.loadCss(Render.Document, dependLoader.getDependencyFiles(Render.Dependency, 'css', 'markdown'));

        dependLoader.loadJs(Render.Document, dependLoader.getDependencyFiles(Render.Dependency, 'js', 'markdown'), function () {
            Render.markdownConvert({});
            if (isMathjax) {
                Render.mathJaxRender();
            }
        });
    },
    mathJax: function (callback) {
        if (callback) {
            Render.callback.mathJax = Render.addCb(defalutCB.mathJax, callback);
        }
        Render.mathJaxRender();
    }
};

var Render = {
    Utils: utils,
    Win: null,
    Document: null,
    Dependency: null,
    callback: {
        markdown: null,
        mathJax: null
    },
    addCb: function (defaultCb, newCb) {
        if (newCb) {
            return function () {
                defaultCb.apply(this, arguments);
                newCb.apply(this, arguments);
            };
        } else {
            return defaultCb;
        }
    },
    cb: function (callback, params) {
        if (callback) {
            callback.apply(this, params ? params : []);
        }
    },
    getBodyTxt: function (body) {
        patchForSafari();

        var text = body.innerText;
        if (!text) {
            // FF自己解析innerText
            text = Render.Utils.getInnerText(body);
        }
        //清理特殊字符
        text = text.replace(String.fromCharCode(65279), '');

        // 替换unicode160的空格为unicode为32的空格，否则pagedown无法识别
        return text.replace(/\u00a0/g, " ");
    },
    markdownConvert: function (frame) {
        var start, end, last, blocks, math, braces,
            inline = false;
        var SPLIT = /(\$\$?|\\(?:begin|end)\{[a-z]*\*?\}|\\[\\{}$]|[{}]|(?:\n\s*)+|@@\d+@@)/i;

        var $doc = $(Render.Document);
        var $body = (frame.container) ? frame.container : $doc.find('body');

        $body.addClass('markdown-body');
        var converter = new markdownBase.Converter({
            nonAsciiLetters: true,
            asteriskIntraWordEmphasis: true
        });

        var text;
        try {
            Render.Utils.markdownPreProcess($body[0]);
            text = Render.tocReady(Render.getBodyTxt($body[0]));
            // console.log(text);

            // 判断代码段数量，超过 50个不使用 codeMirror
            var codeReg = /^```/gm;
            var codeCount = text.match(codeReg);
            if (codeCount) {
                codeCount = codeCount.length / 2;
            } else {
                codeCount = 0;
            }
            markdownExtra.init(converter, {extensions: "all", highlighter: codeCount > 50 ? "prettify" : "codeMirror"});
            // markdownExtra.init(converter, {extensions: "all", highlighter: "prettify"});
            // markdownExtra.init(converter, {extensions: "all", highlighter: "codeMirror"});

            // 判断是否含有mathjax语法
            var judgeMathjaxText = text.replace(/\n/g, '\\n').replace(/\r\n?/g, "\n").replace(/```(.*\n)+?```/gm, '');
            isMathjax = /(\$\$?)[^$\n]+\1/.test(judgeMathjaxText);

            if (isMathjax) {
                text = removeMath(text);
            }

            text = converter.makeHtml(text);
            if (isMathjax) {
                text = replaceMath(text);
            }
            text = Render.xssFilter(text);
            $body[0].innerHTML = text;

        } catch (e) {
            console.error(e);
        }
        try {
            Render.cb(Render.callback.markdown, [isMathjax]);
        } catch (e) {
            console.error(e);
        }


        function replaceMath (text) {
            text = text.replace(/@@(\d+)@@/g, function (match, n) {
                return math[n];
            });
            math = null;
            return text;
        }

        function processMath (i, j) {
            var block = blocks.slice(i, j + 1).join("");
            // 避免 “`$(...)` ... `$(...)`” 这种情况， 变为 “$(...)` ... `$(...)” 整个被处理为重点区块
            if (!block.match(/`/i)) {
                block = block.replace(/&/g, "&amp;") // use HTML entity for &
                    .replace(/</g, "&lt;")  // use HTML entity for <
                    .replace(/>/g, "&gt;"); // use HTML entity for
                while (j > i) {
                    blocks[j] = "";
                    j--;
                }
                blocks[i] = "@@" + math.length + "@@";
                math.push(block);
            }
            start = end = last = null;
        }

        function removeMath (text) {
            start = end = last = null; // for tracking math delimiters
            math = []; // stores math strings for latter

            blocks = text.replace(/\r\n?/g, "\n").split(SPLIT);
            var block;
            for (var i = 1, m = blocks.length; i < m; i += 2) {
                block = blocks[i];
                if (inline && block.match(/\n|`/)) {
                    start = end = last = null;

                } else if (block.charAt(0) === "@") {

                    blocks[i] = "@@" + math.length + "@@";
                    math.push(block);
                } else if (start) {

                    if (block === end) {
                        if (braces) {
                            last = i;
                        } else {
                            processMath(start, i);
                        }
                    } else if (block.match(/\n.*\n/)) {
                        if (last) {
                            i = last;
                            processMath(start, i);
                        }
                        start = end = last = null;
                        braces = 0;
                    } else if (block === "{") {
                        braces++;
                    } else if (block === "}" && braces) {
                        braces--;
                    }
                } else {
                    //
                    // Look for math start delimiters and when
                    // found, set up the end delimiter.
                    //
                    if (/^\$\$?$/.test(block)) {
                        inline = block.length < 2;
                        start = i;
                        end = block;
                        braces = 0;
                    } else if (block.substr(1, 5) === "begin") {
                        start = i;
                        end = "\\end" + block.substr(6);
                        braces = 0;
                    }
                }
            }
            if (last) {
                processMath(start, last);
            }
            return blocks.join("");
        }
    },
    tocReady: function (markdownStr) {
        return markdownStr.replace(/(^[ ]*)\[toc\]([ ]*(\n|$))/igm, '$1[](' + WizToc + ')$2');
    },
    tocRender: function () {
        var container = Render.Document.body;
        var tocHtml = [], min = 6;
        $('h1,h2,h3,h4,h5,h6', container).each(function (index, item) {
            var n = parseInt(item.tagName.charAt(1), 10);
            min = Math.min(min, n);
        });
        $('h1,h2,h3,h4,h5,h6', container).each(function (index, item) {
            var text = (item.textContent || item.innerText).replace(/\(\)<> '"/g, '');
            var id = text;
            var n = parseInt(item.tagName.charAt(1), 10);
            var $item = $(item);
            $item.attr('id', id);
            tocHtml.push('<a class="wiz_toc ' + 'h' + (n - min + 1) + '" href="#' + id + '">' + $item.text() + '</a>');
        });
        tocHtml = '<div class="wiz_toc_layer">' + tocHtml.join('<br/>') + '</div>';

        $('a', container).each(function (index, item) {
            item = $(item);
            if (item.attr('href') == WizToc) {
                item.before(tocHtml);
            }
        });
    },
    flowRender: function () {
        var f = $('.language-flow', Render.Document.body).parents('pre');
        f.each(function (fIndex, fObj) {
            var id = 'wiz-flow-' + fIndex;
            var flowStr = $('textarea', fObj).val();
            if (flowStr.length > 0) {
                try {
                    fObj.style.display = 'none';
                    var diagram = Render.Win.flowchart.parse(flowStr);
                    var flowLayer = Render.Document.createElement('div');
                    flowLayer.id = id;
                    fObj.parentNode.insertBefore(flowLayer, fObj);
                    diagram.drawSVG(id);

                    //修正 svg 保证手机端自动适应大小
                    if (ENV.client.type.isPhone) {
                        //pc、mac 客户端 取消height 设置后， 会导致height 变为0，从而不显示
                        var s = $('svg', flowLayer);
                        if (s.attr('width')) {
                            s.css({
                                'max-width': s.attr('width')
                            }).attr({
                                'height': null,
                                'width': '95%'
                            });
                        }
                    }
                } catch (e) {
                    console.error(e);
                }
            }
        });
    },
    sequenceRender: function () {
        var f = $('.language-sequence', Render.Document.body).parents('pre');
        f.each(function (fIndex, fObj) {
            var id = 'wiz-sequence-' + fIndex;
            var seqStr = $('textarea', fObj).val();
            if (seqStr.length > 0) {
                try {
                    fObj.style.display = 'none';
                    var diagram = Render.Win.Diagram.parse(seqStr);
                    var seqLayer = Render.Document.createElement('div');
                    seqLayer.id = id;
                    fObj.parentNode.insertBefore(seqLayer, fObj);
                    diagram.drawSVG(id, {theme: 'simple'});

                    //修正 svg 保证手机端自动适应大小
                    if (ENV.client.type.isPhone) {
                        //pc、mac 客户端 取消height 设置后， 会导致height 变为0，从而不显示
                        var s = $('svg', seqLayer);
                        if (s.attr('width')) {
                            s.get(0).setAttribute('viewBox', '0 0 ' + s.attr('width') + ' ' + s.attr('height'));
                            s.css({
                                'max-width': s.attr('width')
                            }).attr({
                                'preserveAspectRatio': 'xMidYMid meet',
                                'height': null,
                                'width': '95%'
                            });
                        }
                    }
                } catch (e) {
                    console.error(e);
                }
            }
        });
    },
    mathJaxRender: function () {
        var config = 'MathJax.Hub.Config({\
                            skipStartupTypeset: true,\
                            "HTML-CSS": {\
                                preferredFont: "TeX",\
                                availableFonts: [\
                                    "STIX",\
                                    "TeX"\
                                ],\
                                linebreaks: {\
                                    automatic: true\
                                },\
                                EqnChunk: 10,\
                                imageFont: null\
                            },\
                            SVG: { linebreaks: { automatic: true } },\
                            tex2jax: {\
                                inlineMath: [["$","$"],["\\\\\\\\(","\\\\\\\\)"]],\
                                displayMath: [["$$","$$"],["\\\\[","\\\\]"]],\
                                processEscapes: true },\
                            TeX: {\
                                equationNumbers: {\
                                    autoNumber: "AMS"\
                                },\
                                noUndefined: {\
                                    attributes: {\
                                        mathcolor: "red",\
                                        mathbackground: "#FFEEEE",\
                                        mathsize: "90%"\
                                    }\
                                },\
                                Safe: {\
                                    allow: {\
                                        URLs: "safe",\
                                        classes: "safe",\
                                        cssIDs: "safe",\
                                        styles: "safe",\
                                        fontsize: "all"\
                                    }\
                                }\
                            },\
                            messageStyle: "none"\
                        });';

        scriptLoader.appendJsCode(Render.Document, 'MathJax = null', 'text/javascript');
        scriptLoader.appendJsCode(Render.Document, config, 'text/x-mathjax-config');
        dependLoader.loadJs(Render.Document, dependLoader.getDependencyFiles(Render.Dependency, 'js', 'mathJax'), _render);

        function _render () {
            Render.Win._wizMathJaxCallback = function () {
                Render.cb(Render.callback.mathJax);
            };
            var runMath = 'MathJax.Hub.Queue(' +
                '["Typeset", MathJax.Hub, document.body, _wizMathJaxCallback]);';
            scriptLoader.appendJsCode(Render.Document, runMath, 'text/javascript');
        }
    },
    xssFilter: xssUtils.xssFilter
};
module.exports = MarkdownRender;


},{"../common/dependLoader":19,"../common/env":20,"../common/scriptLoader":23,"../common/utils":24,"../domUtils/domBase":28,"../domUtils/xssUtils":31,"./Markdown.Converter":44,"./Markdown.Extra":45}],47:[function(require,module,exports){
/**
 * 夜间模式的基本方法集合
 */

var ENV = require('../common/env'),
    CONST = require('../common/const'),
    wizStyle = require('../common/wizStyle'),
    domUtils = require('../domUtils/domBase');

var _color = '#7990b6',
    _bk_color = '#1f2126',
    _brightness = '50%',
    _style_id = 'wiz_night_mode_style';

var nightModeUtils = {
    on: function (color, bgColor, brightness) {
        if (color) {
            _color = color;
        }
        if (bgColor) {
            _bk_color = bgColor;
        }
        if (brightness) {
            _brightness = brightness;
        }

        nightModeUtils.off();

        var map = {},
            arr = [];

        checkElement('', ENV.doc.body, map);

        var baseStyle = '{' +
            'color:' + _color + ' !important; ' +
            'background-color:' + _bk_color + ' !important; ' +
            'background-image: none !important; ' +
            'box-shadow: none !important; ' +
            'border-color:' + _color + ' !important; ' +
            '}';

        for (var key in map) {
            if (map.hasOwnProperty(key)) {
                arr.push(key);
            }
        }

        var cssText = arr.join(", ");
        cssText += baseStyle;
        //image brightness
        cssText += 'img{filter: brightness(' + _brightness + ');-webkit-filter: brightness(' + _brightness + ');}';
        cssText += CONST.TAG.TMP_HIGHLIGHT_TAG + '{background-color: #50a9fb !important;  color: black !important;}';

        wizStyle.insertStyle({
            id: _style_id,
            name: CONST.NAME.TMP_STYLE
        }, cssText);

    },
    off: function () {
        var style = ENV.doc.getElementById(_style_id);
        if (style) {
            style.remove();
        }
    }
};

function checkElement(pId, e, map) {
    addItemAttrToMap(pId, e, map);
    var elements = e.children;
    for (var i = 0; i < elements.length; i++) {
        var child = elements[i];
        checkElement((e.id ? e.id : pId), child, map);
    }
}

function addItemAttrToMap(pId, e, map) {
    if (!e)
        return;
    var tagName = e.tagName;

    if (/^(style|script|link|meta|img)$/ig.test(tagName)) {
        return;
    }

    //todo 的图片需要排除
    if (domUtils.hasClass(e, CONST.CLASS.TODO_CHECKBOX) || domUtils.hasClass(e, CONST.CLASS.TODO_AVATAR)) {
        return;
    }


    var className = e.className;
    if (className && className.length > 0) {
        var arr = className.split(" ");
        for (var i = 0; i < arr.length; i++) {
            var name = arr[i];
            if (name.length === 0) {
                continue;
            }
            //if (!!pId) {
            //    addKeyToMap('#' + pId + " ." + name, map);
            //} else {
            addKeyToMap("." + keyFilter(name), map);
            //}
        }
    }
    var id = e.id;
    if (id && id.length > 0) {
        addKeyToMap("#" + keyFilter(id), map);
    }
    //某些页面的控件给自己的特殊样式添加 !important，这些控件一般会在顶层 dom 设置 id ，所以都加上 id
    //为了减少 样式冗余，目前只给 tag 添加 id ， className 暂时不添加
    if (!!pId) {
        addKeyToMap('#' + keyFilter(pId) + " " + (tagName), map);
    } else {
        addKeyToMap(keyFilter(tagName), map);
    }
}
function keyFilter(key) {
    return key.replace(/ /g, '');
}
function addKeyToMap(key, map) {
    //只保留 非数字开头的 且 全部内容为 数字、英文字母、. - _ 的 key
    if (!map[key] && !/^(\.|#)?[\d]+/i.test(key) && /^(\.|#)?[\. \w-]+$/i.test(key)) {
        map[key] = "";
    }
}

module.exports = nightModeUtils;

},{"../common/const":18,"../common/env":20,"../common/wizStyle":25,"../domUtils/domBase":28}],48:[function(require,module,exports){
/**
 * 范围操作的基本方法集合
 */

var ENV = require('./../common/env'),
  CONST = require('./../common/const'),
  base64 = require('./../common/base64'),
  domUtils = require('./../domUtils/domBase');

var lastOrientation = -1;
var fixScrollTimer;

//通用方法集合
var rangeUtils = {
  /**
   * 设置 光标到可视范围内（移动滚动条）
   */
  caretFocus: function () {
    //getClientRects 方法 在 ios 的 safari 上 还有问题
    var range = rangeUtils.getRange(),
      rectList = range ? range.getClientRects() : null,
      rect = (rectList && rectList.length > 0) ? rectList[0] : null,
      cH = ENV.doc.documentElement.clientHeight,
      cW = ENV.doc.documentElement.clientWidth;

    if (rect && rect.top < 0) {
      ENV.doc.body.scrollTop += rect.top;
    } else if (rect && (rect.top + rect.height) > cH) {
      ENV.doc.body.scrollTop += (rect.top + rect.height - cH);
    }

    if (rect && rect.left < 0) {
      ENV.doc.body.scrollLeft += rect.left;
    } else if (rect && (rect.left + rect.width) > cW) {
      ENV.doc.body.scrollLeft += (rect.left + rect.width - cW);
    }
  },
  /**
   * 清除 光标位置的 空字符
   */
  clearFillCharByCollapsed: function () {
    var range = rangeUtils.getRange(),
      txtNode, txtPre, txtNext;
    if (range && range.collapsed && range.startContainer.nodeType === 3) {
      txtNode = range.startContainer;
      txtPre = txtNode.nodeValue.substr(0, range.startOffset);
      txtNext = txtNode.nodeValue.substr(range.startOffset);
      txtPre = txtPre.replace(CONST.FILL_CHAR_REG, '');
      txtNext = txtNext.replace(CONST.FILL_CHAR_REG, '');
      txtNode.nodeValue = txtPre + txtNext;
      rangeUtils.setRange(txtNode, txtPre.length);
    }
  },
  /**
   * 修正滚动条，让光标进入可视区（避免 js 移动光标后，光标不在可视区）
   */
  fixScroll: function () {
    if (fixScrollTimer) {
      clearTimeout(fixScrollTimer);
    }
    fixScrollTimer = setTimeout(_fixScroll, 30);

    function _fixScroll() {
      var rect, winSize, scrollTop, scrollLeft;

      var range = rangeUtils.getRange();
      // range 不存在 或 在 移动设备上有选择区域时（肯定在可视区），不进行修正
      if (!range ||
        (!range.collapsed && (ENV.client.type.isPhone || ENV.client.type.isPad))) {
        return;
      }

      // Iphone 键盘高度会变化（普通键盘 & 九宫格键盘）会导致光标计算错误，
      // 已经提供新方法 setWebViewSizeForFixScroll 所以此部分代码暂时屏蔽
      // var callbackId;
      // // IOS 获取的 数据不正常，需要从客户端获取 可视区高度范围进行修正
      // if (ENV.client.type.isIOS &&
      //     (!ENV.options.ios.webViewHeight || lastOrientation != window.orientation)) {
      //     callbackId = '_' + CONST.CLIENT_EVENT.WizGetWebViewSize + new Date().valueOf();
      //     ENV.win[callbackId] = function (data) {
      //         data = JSON.parse(base64.decode(data));
      //         ENV.options.ios.webViewHeight = data.height;
      //         ENV.options.ios.toolbarHeight = data.toolbarHeight;
      //         lastOrientation = window.orientation;
      //         delete ENV.win[callbackId];
      //         rangeUtils.fixScroll();
      //     };
      //     ENV.client.sendCmdToWiznote(CONST.CLIENT_EVENT.WizGetWebViewSize, {
      //         callback: callbackId
      //     });
      //     return;
      // }

      // IOS 替换内核后，不再从外部传入 webViewHeight / toolbarHeight
      // var webViewHeight = ENV.options.ios.webViewHeight,
      //   toolbarHeight = ENV.options.ios.toolbarHeight;
      // console.log('webViewHeight: ' + webViewHeight + ', toolbarHeight: ' + toolbarHeight);

      // 修改 range 后，一定要让光标移动到可视范围内
      rect = rangeUtils.getRangeClientRect();
      if (!rect) {
        return;
      }
      winSize = domUtils.getWindowSize();
      var webViewHeight = winSize.height, toolbarHeight = 0;

      if (ENV.client.type.isIOS) {
        // winSize.height = webViewHeight - toolbarHeight;
        webViewHeight = winSize.height;
      }

      scrollTop = ENV.doc.body.scrollTop;
      scrollLeft = ENV.doc.body.scrollLeft;
      // console.log(rect.top + '+' + rect.height + '>' + winSize.height);
      // alert(rect.top + '+' + rect.height + '>' + winSize.height);
      if (ENV.client.type.isIOS && rect.top + toolbarHeight < 0) {
        ENV.doc.body.scrollTop = scrollTop + rect.top + toolbarHeight;
      } else if (ENV.client.type.isIOS && rect.top + rect.height > webViewHeight - toolbarHeight) {
        // alert(rect.top + '+' + rect.height + '+' + toolbarHeight + '+' + webViewHeight + '>' + (webViewHeight - toolbarHeight));
        ENV.doc.body.scrollTop = scrollTop + rect.top + rect.height + toolbarHeight - webViewHeight;
      } else if (!ENV.client.type.isIOS && rect.top < 0) {
        ENV.doc.body.scrollTop = scrollTop + rect.top;
      } else if (!ENV.client.type.isIOS && rect.top + rect.height > winSize.height) {
        ENV.doc.body.scrollTop = scrollTop + rect.top + rect.height - winSize.height;
      }
      if (rect.left < scrollLeft) {
        ENV.doc.body.scrollLeft = scrollLeft + rect.left;
      } else if (rect.left + rect.width > winSize.width) {
        ENV.doc.body.scrollLeft = scrollLeft + rect.left + rect.width - winSize.width;
      }
    }
  },
  /**
   * 为 复制/剪切 操作，准备 fragment
   */
  getFragmentForCopy: function () {
    var range = rangeUtils.getRange(),
      fragment = null;
    //无光标时， 不操作任何内容
    if (!range || range.collapsed) {
      return fragment;
    }

    fragment = ENV.doc.createElement('div');
    fragment.appendChild(range.cloneContents());
    domUtils.fragmentFilterForCopy(fragment);
    domUtils.css(fragment, {
      position: 'absolute',
      top: '-99999px',
      left: '-99999px',
      overflow: 'hidden'
    });
    // fragment 必须要加入 body 内，否则有可能会导致 换行符丢失
    ENV.doc.body.appendChild(fragment);
    return fragment;
  },
  /**
   * 获取 Range 对象
   * @returns {*}
   */
  getRange: function () {
    var sel = ENV.doc.getSelection();
    return sel.rangeCount > 0 ? sel.getRangeAt(0) : null;
  },
  /**
   * 获取当前光标所在位置的dom元素
   * isCollapse = true 时 获取光标后面的第一个dom，如果当前在 textNode 内， 则直接返回 textNode
   * isCollapse = false， isBackward = false 获取 光标区间第一个 dom
   * isCollapse = false， isBackward = true 获取 光标区间最后一个 dom
   * @param isBackward
   * @returns {*}
   */
  getRangeAnchor: function (isBackward) {
    var range = rangeUtils.getRange();
    if (!range) {
      return null;
    }
    var rangeContainer = isBackward ? range.startContainer : range.endContainer,
      rangeOffset = isBackward ? range.startOffset : range.endOffset;

    if (!range.collapsed && !isBackward) {
      if (rangeContainer.nodeType === 3 && rangeOffset > 0) {
        return rangeContainer;
      } else if (rangeContainer.nodeType === 3) {
        return domUtils.getPreviousNode(rangeContainer, false, null);
      }

      if (rangeOffset > 0) {
        return domUtils.getLastDeepChild(rangeContainer.childNodes[rangeOffset - 1]);
      } else {
        return domUtils.getPreviousNode(rangeContainer, false, null);
      }
    }

    if (rangeContainer.nodeType === 3 && rangeOffset < rangeContainer.nodeValue.length) {
      return rangeContainer;
    } else if (rangeContainer.nodeType === 3) {
      return domUtils.getNextNode(rangeContainer, false, null);
    }

    if (rangeContainer.childNodes.length === 0) {
      return rangeContainer;
    } else if (rangeOffset == rangeContainer.childNodes.length) {
      return domUtils.getNextNode(rangeContainer.childNodes[rangeOffset - 1], false, null);
    } else {
      return domUtils.getFirstDeepChild(rangeContainer.childNodes[rangeOffset]);
    }

  },
  /**
   * 获取 光标矩形区域 & 位置
   * @returns {*}
   */
  getRangeClientRect: function () {
    var range = rangeUtils.getRange();
    if (!range) {
      return null;
    }

    var rects, rect = range.getBoundingClientRect(), start;
    if (rect.width === 0 && rect.height === 0) {
      // mac and ios safari 使用 range.getBoundingClientRect 经常会得到 0
      rects = rangeUtils.getRange().getClientRects();
      if (rects.length > 0) {
        //修正 光标进入可视区 对于移动设备 只考虑光标折叠的情况
        rect = rects[0];
      }
    }

    // 光标在 input、image 等 DOM 后面时，得到的数据全部为 0 必须修正
    if (rect.width === 0 && rect.height === 0) {
      start = range.startContainer;
      if (start.nodeType === 1) {
        start = start.childNodes[range.startOffset === start.childNodes.length ? range.startOffset - 1 : range.startOffset];
        if (start.getBoundingClientRect) {
          rect = start.getBoundingClientRect();
          rect = {
            bottom: rect.bottom,
            height: rect.height,
            left: rect.left + rect.width,
            right: rect.right,
            top: rect.top,
            width: 0
          };
        }
      }
    }
    return rect;
  },
  /**
   * 根据 startContainer 和 startOffset 获取最终 Dom 元素，
   * 避免 startContainer 为 Element 时，得到的 start 不准确
   * @param startContainer
   * @param startOffset
   * @returns {{startContainer: *, startOffset: *}}
   */
  getRangeDetail: function (startContainer, startOffset) {
    // 必须要保留 isEnd 标志，不能强行设定 nextNode 当作 container
    // 不同的需求会导致有时候需要 光标当前位置的 dom，有时候需要 next dom
    var isEnd = false;
    if (startOffset > 0 && startOffset === domUtils.getEndOffset(startContainer)) {
      // 在 结尾
      if (startContainer.nodeType === 1) {
        startContainer = domUtils.getLastDeepChild(startContainer.childNodes[startOffset - 1]);
        startOffset = domUtils.getEndOffset(startContainer);
      }
      isEnd = true;
    } else if (startContainer.nodeType === 1) {
      if (startContainer.childNodes.length === 0) {
        // br
      } else if (startOffset < startContainer.childNodes.length) {
        startContainer = startContainer.childNodes[startOffset];
        startOffset = 0;
      }
    }

    return {
      container: startContainer,
      offset: startOffset,
      isEnd: isEnd
    }
  },
  /**
   * 根据 获取 光标选中范围内的 dom 集合
   * @param options {noSplit: Boolean}
   * @returns {*}
   */
  getRangeDomList: function (options) {
    var range = rangeUtils.getRange();
    if (!range) {
      return null;
    }
    var startDom = range.startContainer, startOffset = range.startOffset,
      endDom = range.endContainer, endOffset = range.endOffset;
    var list = domUtils.getListA2B({
      startDom: startDom,
      startOffset: startOffset,
      endDom: endDom,
      endOffset: endOffset,
      noSplit: !!options.noSplit
    });
    return list;
  },
  /**
   * 获取 光标范围内 Dom 共同的父节点
   * @returns {*}
   */
  getRangeParentRoot: function () {
    var range = rangeUtils.getRange(), startDom, endDom;
    if (!range) {
      return null;
    }
    startDom = range.startContainer;
    endDom = range.endContainer;
    return domUtils.getParentRoot([startDom, endDom]);
  },
  /**
   * 检验 dom 是否为 selection 的 边缘
   * @param dom
   */
  isRangeEdge: function (dom) {
    var result = {
      isStart: false,
      isEnd: false
    };

    var range = rangeUtils.getRange();
    if (!range) {
      return;
    }
    result.isCollapsed = range.collapsed;
    result.startDom = range.startContainer;
    result.startOffset = range.startOffset;
    result.endDom = range.endContainer;
    result.endOffset = range.endOffset;

    var tmpStartDom, tmpEndDom;
    if (result.startDom.nodeType == 1 && result.startOffset < result.startDom.childNodes.length) {
      tmpStartDom = domUtils.getFirstDeepChild(result.startDom.childNodes[result.startOffset]);
    } else if (result.startDom.nodeType == 1) {
      tmpStartDom = domUtils.getNextNode(result.startDom.childNodes[result.startOffset - 1], false, null);
    }
    if (result.endDom.nodeType == 1 && result.endOffset > 0) {
      tmpEndDom = domUtils.getLastDeepChild(result.endDom.childNodes[result.endOffset - 1]);
    } else if (result.endDom.nodeType == 1) {
      tmpEndDom = domUtils.getPreviousNode(result.endDom, false, null);
    }
    result.isStart = (result.startDom == dom || result.startDom == tmpStartDom);

    result.isEnd = (result.endDom == dom || result.endDom == tmpEndDom);

    return result;
  },
  /**
   * 根据 x，y 坐标移动光标到指定位置
   * @param x
   * @param y
   */
  moveToPoint: function (x, y) {
    var range, textNode, offset;
    if (ENV.doc.caretPositionFromPoint) {
      range = ENV.doc.caretPositionFromPoint(x, y);
      textNode = range.offsetNode;
      offset = range.offset;

    } else if (ENV.doc.caretRangeFromPoint) {
      range = ENV.doc.caretRangeFromPoint(x, y);
      textNode = range.startContainer;
      offset = range.startOffset;
    }
    rangeUtils.setRange(textNode, offset);
  },
  /**
   * 选中指定的 dom 元素
   * @param el
   */
  selectElementContents: function (el) {
    var range = ENV.doc.createRange();
    range.selectNodeContents(el);
    var sel = ENV.doc.getSelection();
    sel.removeAllRanges();
    sel.addRange(range);
  },
  /**
   * 在光标位置选中单个字符，遇到 Fill-Char 特殊字符需要一直选取
   * @param isBackward
   */
  selectCharIncludeFillChar: function (isBackward) {
    var sel = ENV.doc.getSelection(),
      range = sel.getRangeAt(0),
      direction = isBackward ? 'backward' : 'forward';

    var tmpCurDom, tmpOffset, tmpNextDom, s;
    if (range.startContainer.nodeType === 1) {
      tmpCurDom = rangeUtils.getRangeAnchor(false);
      //range.startContainer !== tmpCurDom 的时候， 往往不是在空行的最前面，而是在 前一个 dom 的最后面
      if (range.startContainer == tmpCurDom && domUtils.isTag(tmpCurDom, 'br') && domUtils.isEmptyDom(tmpCurDom.parentNode)) {
        if (tmpCurDom.parentNode.nextSibling) {
          rangeUtils.setRange(tmpCurDom.parentNode, 0, tmpCurDom.parentNode.nextSibling, 0);
        } else {
          sel.modify('move', 'forward', 'character');
          sel.modify('extend', 'backward', 'character');
          if (tmpCurDom.nextSibling) {
            sel.modify('extend', 'backward', 'character');
          }
        }
        return;
      } else if (domUtils.isTag(tmpCurDom, 'br')) {
        sel.modify('extend', direction, 'character');
      }
    }

    sel.modify('extend', direction, 'character');
    range = sel.getRangeAt(0);
    s = range.toString();
    tmpCurDom = rangeUtils.getRangeAnchor(isBackward);

    if (!tmpCurDom) {
      //当没有文字，且只剩下空标签 和 自闭合标签时，有时候会不存在 tmpCurDom
      return;
    }
    if (isBackward && tmpCurDom == range.startContainer) {
      tmpOffset = range.startOffset;
    } else if (!isBackward && tmpCurDom == range.endContainer) {
      tmpOffset = range.endOffset;
    } else {
      //只要 tmpCurDom 不是range 的原始 dom ，就直接设置 tmpOffset 为 -1
      tmpOffset = -1;
    }

    //如果光标在某个 textNode 中间， 则前后都是当前这个 textNode
    if (tmpCurDom.nodeType === 3 && tmpOffset > 0 && tmpOffset < tmpCurDom.nodeValue.length) {
      tmpNextDom = tmpCurDom;
    } else {
      tmpNextDom = isBackward ? domUtils.getPreviousNode(tmpCurDom, false, null) : domUtils.getNextNode(tmpCurDom, false, null);
    }

    if (s.length === 0) {
      //如果当前未选中 自闭合标签（br）且下一个字符是 自闭合标签 则 扩展选中区域
      if (tmpCurDom && !domUtils.isSelfClosingTag(tmpCurDom) && tmpNextDom &&
        (tmpNextDom.nodeType !== 1 ||
          (tmpNextDom.nodeType === 1 && domUtils.isSelfClosingTag(tmpNextDom)))) {
        sel.modify('extend', direction, 'character');
      }
    } else if (s.indexOf(CONST.FILL_CHAR) > -1 && s.replace(CONST.FILL_CHAR_REG, '') === '') {
      //如果当前选中了 文本 但文本未占位字符，则扩展选中区域
      sel.modify('extend', direction, 'character');
    }

  },
  /**
   * 根据 起始 Dom 位置设定 光标选择范围
   * @param start
   * @param startOffset
   * @param end
   * @param endOffset
   */
  setRange: function (start, startOffset, end, endOffset) {
    if (!start && !end) {
      return;
    }
    var maxStart = domUtils.getEndOffset(start),
      maxEnd = domUtils.getEndOffset(end);
    if (startOffset < 0) {
      startOffset = 0;
    } else if (startOffset > maxStart) {
      startOffset = maxStart;
    }
    if (endOffset < 0) {
      endOffset = domUtils.getEndOffset(end);
    } else if (endOffset > maxEnd) {
      endOffset = maxEnd;
    }
    var sel = ENV.doc.getSelection();
    if (!start) {
      start = ENV.doc.body;
      startOffset = 0;
    }
    var range;
    if (sel.rangeCount === 0) {
      range = ENV.doc.createRange();
      range.selectNode(start);
      sel.addRange(range);
    }
    sel.collapse(start, startOffset);
    if (end) {
      sel.extend(end, endOffset);
    }
  },
  /**
   * 专门用于针对 PC 客户端 Chrome 处理 range.insertNode 光标异常
   * @param dom
   */
  setRangeToEnd: function (dom) {
    var target = dom;
    var offset = domUtils.getEndOffset(dom);

    if (domUtils.isSelfClosingTag(target)) {
      target = target.parentNode;
      offset = domUtils.getIndex(dom) + 1;
    }
    rangeUtils.setRange(target, offset);
  }
};

module.exports = rangeUtils;
},{"./../common/base64":17,"./../common/const":18,"./../common/env":20,"./../domUtils/domBase":28}],49:[function(require,module,exports){
/**
 * 范围操作的基本方法集合
 */

var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils'),
    domUtils = require('./../domUtils/domExtend'),
    rangeUtils = require('./rangeBase');

var rangeBackup;
rangeUtils.backupCaret = function () {
    var range = rangeUtils.getRange();
    if (!range) {
        if (rangeBackup) {
            return true;
        }

        domUtils.focus();
        range = rangeUtils.getRange();
        if (!range) {
            return false;
        }
    }
    rangeBackup = rangeUtils.getRange();
    return true;
    //rangeBackup.setEnd(rangeBackup.startContainer, rangeBackup.startOffset);
};

rangeUtils.restoreCaret = function () {
    if (!rangeBackup) {
        return false;
    }
    var sel = ENV.doc.getSelection();
    if (sel.rangeCount == 0) {
        domUtils.focus();
    }
    sel.removeAllRanges();
    sel.addRange(rangeBackup);
    rangeBackup = null;

    return true;
};

/**
 * 在 光标（isCollapse=true）所在位置创建 指定样式的 span
 * make new span when selection's isCollapsed == true
 * @param style
 * @param attr
 */
rangeUtils.modifyCaretStyle = function (style, attr) {
    var sel = ENV.doc.getSelection();
    var focusNode = sel.focusNode;
    var range, key, value, hasSameStyle = true,
        n;

    //get the focus's element.
    if (focusNode.nodeType == 3) {
        focusNode = focusNode.parentNode;
    }
    //check if the current dom is same as the style which is needed.
    for (key in style) {
        if (style.hasOwnProperty(key) && typeof key == 'string') {
            value = style[key];
            if (focusNode.style[key] !== value) {
                hasSameStyle = false;
            }
        }
    }
    if (hasSameStyle) {
        return;
    }

    //if current dom is empty, so don't create span.
    if (domUtils.isTag(focusNode, 'span') && (domUtils.isEmptyDom(focusNode))) {
        domUtils.modifyStyle(focusNode, style, attr);
        n = focusNode;
    } else {
        range = sel.getRangeAt(0);
        range.deleteContents();
        n = domUtils.createSpan();
        n.innerHTML = CONST.FILL_CHAR;
        range.insertNode(n);
        domUtils.modifyStyle(n, style, attr);
    }

    //put the cursor's position to the target dom
    //range = ENV.doc.createRange();
    //range.setStart(n.childNodes[0], 1);
    //range.setEnd(n.childNodes[0], 1);

    //clear redundant span & TextNode
    //var p = focusNode;
    var p = focusNode.parentNode ? focusNode.parentNode : focusNode;
    domUtils.clearChild(p, [n]);

    //reset the selection's range
    rangeUtils.setRange(n.childNodes[0], 1, n.childNodes[0], 1);
    //sel.removeAllRanges();
    //sel.addRange(range);
};
rangeUtils.modifyDomsStyle = function (domList, style, attr, excludeList) {
    //modify style
    domUtils.modifyNodesStyle(domList, style, attr);
    //clear redundant span & TextNode
    var ps = [], i, j, t, tempAmend;
    for (i = 0, j = domList.length; i < j; i++) {
        t = domList[i].parentNode;
        if (!t) {
            continue;
        }
        if (ps.indexOf(t) < 0) {
            ps.push(t);
        }
    }
    //获取需要重构的 dom 集合共同的 parent 节点
    t = domUtils.getParentRoot(ps);
    //如果是 修订节点，则找到修订节点的 父节点进行清理操作
    tempAmend = domUtils.getWizAmendParent(t);
    t = tempAmend ? tempAmend.parentNode : t;
    domUtils.clearChild(t, excludeList);
};
/**
 * 在 光标（isCollapse=false）选择范围内修改所有 dom内容，设置为指定样式
 * modify the style when selection's isCollapsed == false
 * @param style
 * @param attr
 */
rangeUtils.modifyRangeStyle = function (style, attr) {
    var rangeResult, rangeList, rangeLength;
    //get the RangeList
    rangeResult = rangeUtils.getRangeDomList({
        noSplit: false
    });
    if (!rangeResult) {
        return;
    }
    rangeList = rangeResult.list;
    rangeLength = rangeList.length;
    if (rangeLength === 0) {
        return;
    }

    //modify style
    rangeUtils.modifyDomsStyle(rangeList, style, attr,
        [rangeResult.startDomBak, rangeResult.endDomBak]);

    rangeUtils.fixRange(rangeResult);

};
/**
 * 获取 RangeDomList 后修正 Range
 * @param rangeResult
 */
rangeUtils.fixRange = function (rangeResult) {
    //reset the selection's range
    //自闭合标签 需要特殊处理
    var isStartBak = !rangeResult.startDom.parentNode,
        isEndBak = !rangeResult.endDom.parentNode,
        isSelfCloseEnd = domUtils.isSelfClosingTag(rangeResult.endDom);
    //修正 Bak 的Dom
    if (isStartBak && domUtils.isSelfClosingTag(rangeResult.startDomBak)) {
        rangeResult.startDomBak = domUtils.getNextNode(rangeResult.startDomBak, false, rangeResult.endDomBak);
        rangeResult.startOffsetBak = 0;
    }
    rangeUtils.setRange(isStartBak ? rangeResult.startDomBak : rangeResult.startDom,
        isStartBak ? rangeResult.startOffsetBak : rangeResult.startOffset,
        (isEndBak || isSelfCloseEnd) ? rangeResult.endDomBak : rangeResult.endDom,
        (isEndBak || isSelfCloseEnd) ? rangeResult.endOffsetBak : rangeResult.endOffset);
};
/**
 * 修改 光标范围内的 Dom 样式 & 属性
 * @param style
 * @param attr
 */
rangeUtils.modifySelectionDom = function (style, attr) {
    var range = rangeUtils.getRange();
    if (!range) {
        return false;
    }
    if (range.collapsed) {
        rangeUtils.modifyCaretStyle(style, attr);
    } else {
        rangeUtils.modifyRangeStyle(style, attr);
    }
    return true;
};

module.exports = rangeUtils;
},{"./../common/const":18,"./../common/env":20,"./../common/utils":24,"./../domUtils/domExtend":29,"./rangeBase":48}],50:[function(require,module,exports){
/**
 * 阅读器 基础工具包
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    wizStyle = require('../common/wizStyle'),
    blockCore = require('../blockUtils/blockCore'),
    codeCore = require('../codeUtils/codeCore'),
    domUtils = require('../domUtils/domExtend'),
    imgCore = require('../imgUtils/imgCore'),
    tableCore = require('../tableUtils/tableCore'),
    todoCore = require('../todoUtils/todoCore'),
    readerEvent = require('./readerEvent');

var noteSrc = '';
var reader = {
    init: function () {

    },
    insertDefaultStyle: function (isReplace, customCss) {
        wizStyle.insertDefaultStyle(isReplace, customCss);
    },
    on: function () {
        ENV.readonly = true;
        noteSrc = domUtils.getContentHtml({
            isSaveTemp: true
        });
        // 为避免 markdown、mathjax 渲染时 内容闪动，预先加载时，先让 body 透明，当渲染完毕后再删除该样式
        // 预先保存 笔记内容时，需要强行删除此临时样式，避免切换 编辑 和 阅读模式时出现白屏
        // 删除 STYLE_FOR_LOAD 最终没有放到 callback 内，主要避免 白屏事件过长，所以此方法可屏蔽
        // noteSrc = domUtils.removeStyleByNameFromHtml(noteSrc, [CONST.NAME.STYLE_FOR_LOAD]);
        wizStyle.insertTmpReaderStyle();
    },
    afterRender: function(callback) {
        codeCore.loadDependency(_callback);

        function _callback() {
            // 阅读模式有 markdown、mathjax 渲染，是异步操作，必须要等待其结束后执行
            domUtils.fixOrderList();
            readerEvent.on();
            imgCore.on();
            tableCore.on();
            todoCore.on();
            blockCore.on();
            codeCore.on();

            imgCore.setImgFullPath();

            //禁用 输入框（主要用于 九宫格 处理）
            setDomReadOnly('input', true);
            setDomReadOnly('textarea', true);
            if (typeof callback === 'function') {
                callback();
            }
        }
    },
    off: function () {
        readerEvent.off();
        blockCore.off();
        todoCore.off();
        codeCore.off();
        tableCore.off();
        imgCore.off();

        if (!noteSrc) {
            return;
        }
        if (ENV.options.noteType === CONST.NOTE_TYPE.COMMON) {
            domUtils.removeByName(CONST.NAME.TMP_STYLE);
            domUtils.removeByTag(CONST.TAG.TMP_TAG);
            setDomReadOnly('input', false);
            setDomReadOnly('textarea', false);
        } else {
            ENV.doc.open("text/html", "replace");
            ENV.doc.write(noteSrc);
            ENV.doc.close();
        }
    }
};

function setDomReadOnly(tag, readonly) {
    var domList = ENV.doc.getElementsByTagName(tag),
        i, obj;
    for (i = 0; i < domList.length; i++) {
        obj = domList[i];
        obj.readOnly = readonly;
    }
}

module.exports = reader;

},{"../blockUtils/blockCore":12,"../codeUtils/codeCore":14,"../common/const":18,"../common/env":20,"../common/wizStyle":25,"../domUtils/domExtend":29,"../imgUtils/imgCore":40,"../tableUtils/tableCore":52,"../todoUtils/todoCore":56,"./readerEvent":51}],51:[function(require,module,exports){
/**
 * editor 使用的基本事件处理
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    domUtils = require('../domUtils/domExtend'),
    codeUtils = require('../codeUtils/codeUtils'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    imgUtils = require('../imgUtils/imgUtils');

/**
 * 复制/剪切 选中区域
 * @param e
 * @param isCut
 */
function copySelection(e) {
    var range = rangeUtils.getRange(),
        fragment,
        hasCodeMirror = false, tmp;
    var startContainer, endContainer;

    // 检查复制的区域内是否存在 codeMirror
    if (range && !range.collapsed) {
        // body contenteditable= false 时，可以普通文本和 CodeMirror内文本同时选择，
        // 所以编辑时 和 阅读时 处理规则不一致
        startContainer = codeUtils.getContainerFromChild(range.startContainer);
        endContainer = codeUtils.getContainerFromChild(range.endContainer);

        // 同一个 CodeMirror 内 复制操作 WizEditor 不进行任何干预
        // 移动端 不使用 CodeMirror 内的复制操作（nocursor 会导致复制错误）
        if (startContainer && startContainer === endContainer) {
            if (!ENV.client.type.isPhone && !ENV.client.type.isPad) {
                return;
            } else {
                hasCodeMirror = true;
            }
        } else {
            tmp = range.cloneContents();
            hasCodeMirror = !!tmp.querySelector('.' + CONST.CLASS.CODE_CONTAINER);
        }
    }

    // 只有选中 CodeMirror 区域 才干涉选中内容
    if (!range || range.collapsed || !hasCodeMirror) {
        return;
    }

    fragment = rangeUtils.getFragmentForCopy();

    if (fragment) {
        var text = fragment.innerText;
        var html = fragment.innerHTML;

        fragment.innerHTML = '';
        ENV.doc.body.removeChild(fragment);
        fragment = null;

        e.clipboardData.clearData();
        e.clipboardData.setData('text/plain', text);
        e.clipboardData.setData('text/html', html);

        if (e.clipboardData.getData('text')) {
            utils.stopEvent(e);
            return;
        }

        // hack for ios
        // ios safari 不支持 e.clipboardData.setData
        var hackDiv = ENV.doc.createElement(CONST.TAG.TMP_TAG);
        var field = ENV.doc.createElement('textarea');
        field.value = text;
        field.setAttribute('readonly', '');

        domUtils.css(field, {
            position: 'absolute',
            top: '0',
            left: '0',
            width: '1000px'
        });
        domUtils.css(hackDiv, {
            position: 'absolute',
            top: '-99999px',
            left: '-99999px',
            overflow: 'hidden'
        });
        hackDiv.appendChild(field);
        ENV.doc.body.appendChild(hackDiv);
        field.focus();
        field.selectionStart = 0;
        field.selectionEnd = field.value.length;
        setTimeout(function() {
            ENV.doc.body.removeChild(hackDiv);
            field = null;
            hackDiv = null;
        }, 50);
    }

    fragment = null;

}

var ReaderEvent = {
    on: function () {
        ReaderEvent.bind();
    },
    off: function () {
        ReaderEvent.unbind();
    },
    bind: function () {
        ReaderEvent.unbind();
        ENV.doc.addEventListener('dblclick', handler.onDblclick);
        ENV.doc.addEventListener('click', handler.onClick);
        ENV.doc.addEventListener('copy', handler.onCopy);
        ENV.doc.addEventListener('cut', handler.onCopy);
        ENV.doc.addEventListener('scroll', handler.onScroll);

        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            ENV.doc.addEventListener('touchend', handler.onTouchEnd);
            ENV.doc.addEventListener('touchstart', handler.onTouchStart);
        }
    },
    unbind: function () {
        ENV.doc.removeEventListener('dblclick', handler.onDblclick);
        ENV.doc.removeEventListener('click', handler.onClick);
        ENV.doc.removeEventListener('copy', handler.onCopy);
        ENV.doc.removeEventListener('cut', handler.onCopy);
        ENV.doc.removeEventListener('scroll', handler.onScroll);
        ENV.doc.removeEventListener('touchend', handler.onTouchEnd);
        ENV.doc.removeEventListener('touchstart', handler.onTouchStart);
    }
};

var handler = {
    onDblclick: function (e) {
        ENV.event.call(CONST.EVENT.ON_DBLCLICK, e);
    },
    onClick: function (e) {
        ENV.event.call(CONST.EVENT.ON_CLICK, e);
    },
    onCopy: function (e) {
        copySelection(e);
    },
    onScroll: function (e) {
        ENV.event.call(CONST.EVENT.ON_SCROLL, e);
    },
    onTouchEnd: function (e) {
        ENV.event.call(CONST.EVENT.ON_TOUCH_END, e);
    },
    onTouchStart: function (e) {
        ENV.event.call(CONST.EVENT.ON_TOUCH_START, e);
    }
};

module.exports = ReaderEvent;
},{"../codeUtils/codeUtils":16,"../common/const":18,"../common/env":20,"../common/utils":24,"../domUtils/domExtend":29,"../imgUtils/imgUtils":42,"../rangeUtils/rangeExtend":49}],52:[function(require,module,exports){
/**
 * 表格操作核心包 core
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    blockUtils = require('../blockUtils/blockUtils'),
    codeUtils = require('../codeUtils/codeUtils'),
    domUtils = require('../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    tableUtils = require('./tableUtils'),
    tableMenu = require('./tableMenu'),
    tableZone = require('./tableZone');


//TODO 所有配色 要考虑到 夜间模式

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_DRAG_START, _event.handler.onDragStart);
        ENV.event.add(CONST.EVENT.ON_KEY_UP, _event.handler.onKeyUp);

        if (ENV.client.type.isPhone || ENV.client.type.isPad) {
            ENV.event.add(CONST.EVENT.ON_TOUCH_START, _event.handler.onMouseDown);
            ENV.event.add(CONST.EVENT.ON_TOUCH_END, _event.handler.onMouseUp);
        } else {
            ENV.event.add(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
            ENV.event.add(CONST.EVENT.ON_MOUSE_OVER, _event.handler.onMouseOver);
            ENV.event.add(CONST.EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
        }
        ENV.event.add(CONST.EVENT.ON_SELECTION_CHANGE, _event.handler.onSelectionChange);
        ENV.event.add(CONST.EVENT.AFTER_RESTORE_HISTORY, _event.handler.afterRestoreHistory);
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_DRAG_START, _event.handler.onDragStart);
        ENV.event.remove(CONST.EVENT.ON_KEY_UP, _event.handler.onKeyUp);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_DOWN, _event.handler.onMouseDown);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_OVER, _event.handler.onMouseOver);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_UP, _event.handler.onMouseUp);
        ENV.event.remove(CONST.EVENT.ON_TOUCH_START, _event.handler.onMouseDown);
        ENV.event.remove(CONST.EVENT.ON_TOUCH_END, _event.handler.onMouseUp);
        ENV.event.remove(CONST.EVENT.ON_SELECTION_CHANGE, _event.handler.onSelectionChange);
        ENV.event.remove(CONST.EVENT.AFTER_RESTORE_HISTORY, _event.handler.afterRestoreHistory);
    },
    handler: {
        afterRestoreHistory: function () {
            //恢复历史后，需要修正 zone
            var tmpCells, cells = [], cell,
                i, j;

            tmpCells = ENV.doc.getElementsByClassName(CONST.CLASS.SELECTED_CELL);

            if (tmpCells.length === 0) {
                tableZone.clear();
                return;
            }

            for (i = 0, j = tmpCells.length; i < j; i++) {
                cells.push(tmpCells[i]);
            }

            var table = domUtils.getParentByTagName(cells[0], 'table', true, null);
            if (!table) {
                tableZone.clear();
                return;
            }

            tableZone.setStart(cells[0]);

            var zone = tableZone.getZone();
            var endCell = cells[cells.length - 1],
                endCellRange = tableUtils.getRangeByCellData(tableUtils.getCellData(zone.grid, endCell)),
                cellRange;

            for (i = 1; i < cells.length - 1; i++) {
                cell = cells[i];
                if (cell.rowSpan == 1) {
                    continue;
                }
                cellRange = tableUtils.getRangeByCellData(tableUtils.getCellData(zone.grid, cell));
                if (cellRange.maxY > endCellRange.maxY || (cellRange.maxY = endCellRange.maxY && cellRange.maxX > endCellRange.maxX)) {
                    endCell = cell;
                    endCellRange = cellRange;
                }
            }

            tableZone.setEnd(endCell);

            //修正 Menu
            tableMenu.show();
        },
        onDragStart: function (e) {
            //表格内禁止拖拽操作
            var table = domUtils.getParentByTagName(e.target, 'table', true, null);
            if (table) {
                utils.stopEvent(e);
            }
        },
        onKeyDown: function (e) {
            var sel = ENV.doc.getSelection();
            var zone = tableZone.getZone();
            if (!zone.range) {
                return true;
            }

            var method = e.shiftKey ? 'extend' : 'move';
            var code = e.keyCode || e.which,
                direct,
                charMove = false,
                oldCur = sel.focusNode,
                newCur, startX;
            switch (code) {
                case 37:
                    //left
                    if (!e.ctrlKey && !e.metaKey && oldCur) {
                        charMove = true;
                        sel.modify(method, 'backward', 'character');
                    }
                    direct = {x: -1, y: 0};
                    break;
                case 38:
                    //up
                    if (!e.ctrlKey && !e.metaKey && oldCur) {
                        charMove = true;
                        sel.modify('move', 'backward', 'line');
                    }
                    direct = {x: 0, y: -1};
                    break;
                case 9:
                    //Tab
                    if (!e.shiftKey) {
                        direct = {x: 1, y: 0, canChangeRow: true};
                    }
                    break;
                case 39:
                    //right
                    if (!e.ctrlKey && !e.metaKey && oldCur) {
                        charMove = true;
                        sel.modify(method, 'forward', 'character');
                    }
                    direct = {x: 1, y: 0};
                    break;
                case 13:
                    //Enter
                    if (!e.ctrlKey && !e.metaKey) {
                        break;
                    }

                case 40:
                    //down
                    if (!e.ctrlKey && !e.metaKey && oldCur) {
                        charMove = true;
                        sel.modify('move', 'forward', 'line');
                    }
                    direct = {x: 0, y: 1};
                    break;
            }

            var last, cellData, check, codeContainer, cm;

            if (charMove) {
                newCur = sel.focusNode;
                oldCur = domUtils.getParentByTagName(oldCur, ['td', 'th'], true, null);
                newCur = domUtils.getParentByTagName(newCur, ['td', 'th'], true, null);
                if (newCur && newCur != oldCur) {
                    if (code == 38 || code == 40 || code == 13) {
                        charMove = false;
                    } else {
                        tableZone.setStart(newCur).setEnd(newCur);
                    }
                }

                if (charMove) {
                    // 如果光标移动到表格外，则清除 表格选中区域 以及表格工具栏
                    check = tableUtils.checkCaretInTableContainer(e);
                    if (!newCur && !check.tableContainer) {
                        tableZone.clear();
                        tableMenu.show();
                    } else if (check.before) {
                        // 如果移动到表格最前面，设置光标为单元格最前面
                        rangeUtils.setRange(zone.start.cell, 0);
                        tableZone.setStart(zone.start.cell).setEnd(zone.start.cell);
                    }

                    utils.stopEvent(e);
                    return false;
                } else if (code == 38 && zone.start.cell == zone.end.cell && zone.start.y_src == 0) {
                    // 表格第一行 按 up 键，直接进入 table 上面的段落
                    startX = zone.start.x;
                    newCur = domUtils.getPreviousNode(tableUtils.getContainer(oldCur));
                    while (newCur && !domUtils.isTag(newCur, 'br') && domUtils.isEmptyDom(newCur)) {
                        newCur = domUtils.getPreviousNode(newCur);
                    }
                    if (newCur) {
                        // 如果 table 上面的段落不是 table，则 x = 0 的 cell 直接进入行首，否则行尾
                        tableZone.clear();
                        tableMenu.show();
                        codeContainer = codeUtils.getContainerFromChild(newCur);
                        if (codeContainer) {
                            codeUtils.focusToLast(codeContainer.codeMirror);
                        } else {
                            rangeUtils.setRange(newCur, domUtils.getEndOffset(newCur));
                            if (startX === 0 && !domUtils.getParentByTagName(newCur, ['table'], true, null)) {
                                sel.modify('move', 'backward', 'lineboundary');
                            }
                            rangeUtils.fixScroll();
                        }
                    } else {
                        // 如果 table 上面无内容，则保持原样
                        rangeUtils.setRange(zone.start.cell, 0);
                        tableZone.setStart(zone.start.cell).setEnd(zone.start.cell);
                    }
                    utils.stopEvent(e);
                    return false;
                } else if (code == 40 && zone.start.cell == zone.end.cell && zone.start.y_src + zone.start.cell.rowSpan >= zone.grid.length) {
                    // 表格最后一行 按 down 键，直接进入 table 下面的段落
                    startX = zone.start.x;
                    newCur = domUtils.getNextNode(tableUtils.getContainer(oldCur));
                    while (newCur && !domUtils.isTag(newCur, 'br') && domUtils.isEmptyDom(newCur)) {
                        newCur = domUtils.getNextNode(newCur);
                    }
                    if (newCur) {
                        // 如果 table 下面的段落不是 table，则 x = 0 的 cell 直接进入行首，否则行尾
                        tableZone.clear();
                        tableMenu.show();
                        codeContainer = codeUtils.getContainerFromChild(newCur);
                        if (codeContainer) {
                            codeUtils.focusToFirst(codeContainer.codeMirror);
                        } else {
                            rangeUtils.setRange(newCur, domUtils.getEndOffset(newCur));
                            if (startX === 0 && !domUtils.getParentByTagName(newCur, ['table'], true, null)) {
                                sel.modify('move', 'backward', 'lineboundary');
                            }
                            rangeUtils.fixScroll();
                        }
                    } else {
                        // 如果 table 上面无内容，则保持原样
                        tableZone.setStart(zone.start.cell).setEnd(zone.start.cell);
                    }
                    utils.stopEvent(e);
                    return false;
                }
            }

            if (direct) {
                if (e.shiftKey) {
                    last = zone.end || zone.start;
                } else {
                    last = zone.start;
                }

                cellData = tableZone.switchCell(last, direct);
                if (cellData == last) {
                    // 如果目标 cell 不变，则不进行任何处理
                    utils.stopEvent(e);
                    return false;
                } else if (cellData) {
                    if (e.shiftKey) {
                        tableZone.setEnd(cellData.cell, true);
                    } else {
                        // if (direct.x + direct.y > 0) {
                        //     rangeUtils.setRange(cellData.cell, 0);
                        // }
                        tableZone.setStart(cellData.cell, cellData.x, cellData.y).setEnd(cellData.cell);
                    }
                    rangeUtils.fixScroll();
                    utils.stopEvent(e);
                    return false;
                }
            }
            return true;
        },
        onKeyUp: function (e) {
            var zone = tableZone.getZone(),
                range = rangeUtils.getRange(),
                cell;
            //从非表格的地方 用键盘移动光标到 单元格内，直接选中该单元格
            if (!zone.range && range && range.collapsed) {
                cell = domUtils.getParentByTagName(range.startContainer, ['td', 'th'], true, null);
                if (cell) {
                    tableZone.setStart(cell).setEnd(cell);
                    tableMenu.show();
                }
            }

            tableUtils.fixSelection(e);
        },
        onMouseDown: function (e) {
            var isLeft = (e.type !== 'mousedown' || e.button === 0 || e.button === 1);
            if (!isLeft) {
                tableMenu.hide();
                return;
            }

            var isMenu = tableUtils.isMenu(e.target);
            if (isMenu) {
                return;
            }

            var cell = domUtils.getParentByTagName(e.target, ['th', 'td'], true, null);
            var table = cell ? domUtils.getParentByTagName(cell, 'table', false, null) : null;
            var pos = tableUtils.getMousePosition(e, table);
            var isZoneBorder = tableZone.isZoneBorder(e);

            if (isZoneBorder.isBodyBorder || isZoneBorder.isContainer) {
                if (!ENV.client.type.isPhone && !ENV.client.type.isPad) {
                    //手机端不能阻止事件，否则会导致点击这些区域无法滚动屏幕
                    utils.stopEvent(e);
                }
                return;
            }

            if (!ENV.client.type.isPhone && !ENV.client.type.isPad) {
                if (isZoneBorder.isRight) {
                    tableZone.startDragColLine(e.target, pos.x);
                    return;
                }
                if (isZoneBorder.isBottom) {
                    tableZone.startDragRowLine(e.target, pos.y);
                    return;
                }
                if (isZoneBorder.isDot) {
                    return;
                }
            }

            if (isZoneBorder.isBorder || isZoneBorder.isScroll) {
                return;
            }

            tableZone.setStart(cell);
            tableMenu.show();
        },
        onMouseOver: function (e) {
            var end = domUtils.getParentByTagName(e.target, ['td', 'th'], true, null);
            tableZone.modify(end);
        },
        onMouseUp: function (e) {
            tableUtils.fixSelection(e);

            var isLeft = (e.type !== 'mouseup' || e.button === 0 || e.button === 1);
            if (!isLeft) {
                return;
            }
            var isMenu, isZoneBorder;
            var zone = tableZone.getZone();
            //当前正在选择单元格时， 不考虑 up 的位置是否 menu 等
            if (!zone.active) {
                isMenu = tableUtils.isMenu(e.target);
                if (isMenu) {
                    return;
                }

                isZoneBorder = tableZone.isZoneBorder(e);
                if (isZoneBorder.isRight && !tableZone.isRangeActiving()) {
                    return;
                }
                if (isZoneBorder.isBottom && !tableZone.isRangeActiving()) {
                    return;
                }
                if (isZoneBorder.isDot) {
                    return;
                }
                if (isZoneBorder.isBorder || isZoneBorder.isScroll) {
                    return;
                }
            }
            var cell = domUtils.getParentByTagName(e.target, ['td', 'th'], true, null);
            tableZone.setEnd(cell);
            tableMenu.show();
        },
        onSelectionChange: function (e) {
            var check = tableUtils.checkCaretInTableContainer(e);
            var cell;
            // 保证自动修正 cell 后，还能正常显示光标，所以不再判断 zone.table
            // var zone = tableZone.getZone();
            // if (check.tableContainer && zone.table !== check.tableContainer.querySelector('table')) {
            var range = rangeUtils.getRange(), tableContainer;
            if (range && !range.collapsed) {
                tableContainer = tableUtils.getContainer(range.startContainer);
                if (tableContainer) {
                    // 避免拖拽文字进入 tableContainer 边缘
                    domUtils.moveOutFromTableContainer(tableContainer);
                }
                // table container 在 <p> 内时， 拖拽多行文本到 容器末尾，
                // 会导致部分文本在新的 container 内
                tableContainer = tableUtils.getContainer(range.endContainer);
                if (tableContainer) {
                    // 避免拖拽文字进入 tableContainer 边缘
                    domUtils.moveOutFromTableContainer(tableContainer);
                }
            }
            var zone = tableZone.getZone();
            if (check.tableContainer && !zone.range) {
                //如果光标定位在 table & table container 之间，则定位到 table 内第一个 td
                cell = check.tableContainer.querySelectorAll('td');
                cell = (check.tableMenu || check.after) ? cell[cell.length - 1] : (check.before ? cell[0] : null);
                if (cell) {
                    tableZone.setStart(cell).setEnd(cell);
                    tableMenu.show();
                }
            }
        }
    }
};

var tableCore = {
    on: function () {
        if (!ENV.readonly) {
            _event.bind();
            tableMenu.init(tableCore);
        }
        tableUtils.initTableContainer(null);
        tableZone.clear();
    },
    off: function () {
        tableZone.clear();
    },
    canCreateTable: function () {
        return tableUtils.canCreateTable(tableZone.getZone());
    },
    clearCellValue: function () {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        tableUtils.clearCellValue(zone.grid, zone.range);
    },
    deleteCols: function () {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        if (zone.range.minX === 0 && zone.range.maxX === zone.grid[0].length - 1) {
            tableCore.deleteTable();
            return;
        }

        historyUtils.saveSnap(false);
        var i;
        for (i = zone.range.maxX; i >= zone.range.minX; i--) {
            tableUtils.deleteCols(zone.grid, i);
        }
        tableZone.clear();
    },
    deleteRows: function () {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        if (zone.range.minY === 0 && zone.range.maxY === zone.grid.length - 1) {
            tableCore.deleteTable();
            return;
        }

        historyUtils.saveSnap(false);
        var i;
        for (i = zone.range.maxY; i >= zone.range.minY; i--) {
            tableUtils.deleteRows(zone.grid, i);
        }
        tableZone.clear();
    },
    deleteTable: function () {
        var zone = tableZone.getZone();
        if (!zone.table) {
            return;
        }
        historyUtils.saveSnap(false);

        var parent = zone.table.parentNode;
        if (parent) {
            parent.removeChild(zone.table);
        }
        tableMenu.remove();
        tableZone.remove();
        parent = domUtils.getParentByFilter(parent, function (dom) {
            return domUtils.hasClass(dom, CONST.CLASS.TABLE_CONTAINER);
        }, true);

        var enter;
        if (parent) {
            enter = ENV.doc.createElement('br');
            parent.parentNode.insertBefore(enter, parent);
            domUtils.remove(parent);
            rangeUtils.setRange(enter, 0);
        }
    },
    distributeCols: function () {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        tableUtils.distributeCols(zone.table, zone.grid);
        tableZone.updateGrid();
    },
    insertCol: function (before) {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        tableUtils.insertCol(zone.grid, before ? zone.range.minX : zone.range.maxX + 1);
        tableZone.updateGrid();
    },
    insertRow: function (before) {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        tableUtils.insertRow(zone.grid, before ? zone.range.minY : zone.range.maxY + 1);
        tableZone.updateGrid();
    },
    insertTable: function (col, row) {
        historyUtils.saveSnap(false);
        var range = rangeUtils.getRange();
        var tmpCell;

        if (!tableCore.canCreateTable()) {
            return;
        }
        var curObj, container, table;
        if (range) {
            range.deleteContents();
            curObj = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            curObj = domUtils.getBlockParent(curObj.container, true);
            if (curObj && curObj !== ENV.doc.body &&
                domUtils.isEmptyDom(curObj) && domUtils.isTag(curObj, 'div')) {
                container = curObj;
                curObj.innerHTML = '';
            }
        }
        table = tableUtils.createTable(col, row);
        container = container ? container : ENV.doc.createElement('div');
        container.appendChild(table);
        if (!container.parentNode) {
            container = blockUtils.insertBlock(table);
        }
        tableUtils.initTableContainer(table);

        //修正 光标
        tmpCell = table.querySelector('td');
        tableZone.setStart(tmpCell).setEnd(tmpCell);

        ENV.event.call(CONST.EVENT.UPDATE_RENDER);
    },
    merge: function () {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        var cell = tableUtils.mergeCell(zone.grid, zone.range);
        if (cell) {
            tableZone.updateGrid();
            tableZone.setStart(cell).setEnd(cell);
        }
    },
    /**
     * 修改单元格 Dom 样式 & 属性
     * @param style
     * @param attr
     */
    modifySelectionDom: function (style, attr) {
        var range = rangeUtils.getRange(),
            zone = tableZone.getZone();

        if ((!range || range.collapsed) && zone.range) {
            tableUtils.modifySelectionDom(zone, style, attr);
            return true;
        }
        return false;
    },
    onKeyDown: _event.handler.onKeyDown,
    setCellAlign: function (align, valign) {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        tableUtils.setCellAlign(zone.grid, zone.range, {
            align: align,
            valign: valign
        });
        tableZone.setStartRange();
    },
    setCellBg: function (bgColor) {
        var zone = tableZone.getZone();
        if (!zone.range) {
            return;
        }
        historyUtils.saveSnap(false);
        tableUtils.setCellBg(zone.grid, zone.range, bgColor);
        tableZone.setStartRange();
    },
    split: function () {
        var zone = tableZone.getZone();
        var range = tableUtils.splitCell(zone.table, zone.grid, zone.range);
        if (range) {
            historyUtils.saveSnap(false);
            tableZone.updateGrid();
            zone = tableZone.getZone();
            tableZone.setStart(zone.grid[range.minY][range.minX].cell)
                .setEnd(zone.grid[range.maxY][range.maxX].cell);
        }
    }
};

module.exports = tableCore;
},{"../blockUtils/blockUtils":13,"../codeUtils/codeUtils":16,"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"./tableMenu":53,"./tableUtils":54,"./tableZone":55}],53:[function(require,module,exports){
/*
 表格菜单 控制
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    Lang = require('../common/lang'),
    LANG = Lang.getLang(),
    tableUtils = require('./tableUtils'),
    tableZone = require('./tableZone'),
    domUtils = require('../domUtils/domExtend');

var colorPadDemo;
var _id = {
    col: 'wiz-menu-col',
    align: 'wiz-menu-align',
    bg: 'wiz-menu-bg',
    bgDemo: 'wiz-menu-bg-demo',
    cells: 'wiz-menu-cells',
    more: 'wiz-menu-more'
};
var _class = {
    active: 'active',
    disabled: 'disabled',
    clickItem: 'click-item',
    colorPadItem: 'wiz-table-color-pad-item',
    alignItem: 'wiz-table-cell-align-item'
};
var _subType = {
    list: 1,
    custom: 2
};

var tableCore;
var menuObj;

function createMenu() {
    var menu = ENV.doc.querySelector('.' + CONST.CLASS.TABLE_TOOLS);
    if (menu) {
        return menu;
    }

    var menuData = [
        {
            id: _id.col,
            exClass: 'icon-insert editor-icon',
            subMenu: {
                type: _subType.list,
                data: [
                    {
                        type: CONST.TYPE.TABLE.INSERT_ROW_UP,
                        name: LANG.Table.InsertRowUp,
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.INSERT_ROW_DOWN,
                        name: LANG.Table.InsertRowDown,
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.INSERT_COL_LEFT,
                        name: LANG.Table.InsertColLeft,
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.INSERT_COL_RIGHT,
                        name: LANG.Table.InsertColRight,
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.DELETE_ROW,
                        name: LANG.Table.DeleteRow,
                        isSplit: true
                    },
                    {
                        type: CONST.TYPE.TABLE.DELETE_COL,
                        name: LANG.Table.DeleteCol,
                        isSplit: false
                    }
                ]
            }
        },
        {
            id: _id.align,
            exClass: 'icon-align editor-icon',
            subMenu: {
                type: _subType.custom,
                make: function () {
                    var typeList = [
                        ['top', 'middle', 'bottom'], ['left', 'center', 'right']
                    ];
                    var i, j, dataAlignType;
                    var str = '<div class="wiz-table-menu-sub wiz-table-cell-align">';
                    for (i = 0; i < typeList.length; i++) {
                        str += '<div>';
                        for (j = 0; j < typeList[i].length; j++) {
                            dataAlignType = (i === 0) ? 'valign' : 'align';
                            str += '<div class="' + _class.alignItem + ' ' + _class.clickItem +
                                '" data-type="' + CONST.TYPE.TABLE.SET_CELL_ALIGN +
                                '" data-align-type="' + dataAlignType +
                                '" data-align-value="' + typeList[i][j] +
                                '">';
                            if (i === 0) {
                                str += '<i class="editor-icon icon-box"></i>';
                                str += '<i class="editor-icon valign icon-valign_' + typeList[i][j] + '"></i>';
                            } else {
                                str += '<i class="editor-icon align icon-align_' + typeList[i][j] + '"></i>';
                            }

                            str += '</div>';
                        }
                        str += '</div>';
                    }
                    str += '</div>';

                    return str;
                }
            }
        },
        {
            id: _id.bg,
            exClass: 'icon-box editor-icon',
            subMenu: {
                type: _subType.custom,
                make: function () {
                    var colors = [
                        ['', '#f7b6ff', '#fecf9c'],
                        ['#acf3fe', '#b2ffa1', '#b6caff'],
                        ['#ffc7c8', '#eeeeee', '#fef49c']
                    ];
                    var i, j;
                    var str = '<div class="wiz-table-menu-sub wiz-table-color-pad">';
                    for (i = 0; i < colors.length; i++) {
                        str += '<div>';
                        for (j = 0; j < colors[i].length; j++) {
                            str += '<div class="' + _class.colorPadItem + ' ' + _class.clickItem + '" data-color="' + colors[i][j] + '" data-type="' + CONST.TYPE.TABLE.SET_CELL_BG + '">';
                            str += '<i class="editor-icon icon-box"></i>';
                            if (i === 0 && j === 0) {
                                str += '<i class="pad-demo editor-icon icon-oblique_line"></i>';
                            } else {
                                str += '<i class="pad-demo editor-icon icon-inner_box" style="color:' + colors[i][j] + ';"></i>';
                            }
                            str += '</div>';
                        }
                        str += '</div>';
                    }
                    str += '</div>';
                    return str;
                }
            }
        },
        {
            id: _id.cells,
            exClass: 'icon-merge editor-icon',
            subMenu: {
                type: _subType.list,
                data: [
                    {
                        type: CONST.TYPE.TABLE.MERGE_CELL,
                        name: LANG.Table.MergeCell,
                        // exClass: tableZone.isSingleCell() ? 'disabled' : '',
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.SPLIT_CELL,
                        name: LANG.Table.SplitCell,
                        // exClass: tableZone.hasMergeCell() ? '' : 'disabled',
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.CLEAR_CELL,
                        name: LANG.Table.ClearCell,
                        isSplit: false
                    }
                ]
            }
        },
        {
            id: _id.more,
            exClass: 'icon-more editor-icon',
            subMenu: {
                type: _subType.list,
                data: [
                    {
                        type: CONST.TYPE.TABLE.DISTRIBUTE_COLS,
                        name: LANG.Table.DistributeCols,
                        exClass: '',
                        isSplit: false
                    },
                    {
                        type: CONST.TYPE.TABLE.DELETE_TABLE,
                        name: LANG.Table.DeleteTable,
                        exClass: '',
                        isSplit: true
                    }
                ]
            }
        }
    ];

    var i, m;

    menu = ENV.doc.createElement(CONST.TAG.TMP_TAG);
    domUtils.addClass(menu, CONST.CLASS.TABLE_TOOLS);

    var menuHtml = '<ul>';
    for (i = 0; i < menuData.length; i++) {
        m = menuData[i];
        menuHtml += '<li id="' + m.id + '" class="' + CONST.CLASS.TABLE_MENU_ITEM + '">' +
            '<div class="' + CONST.CLASS.TABLE_MENU_BUTTON + '">' +
            '<i class="' + m.exClass + '"></i>';
        if (m.id === _id.bg) {
            menuHtml += '<i id="' + _id.bgDemo + '" class="editor-icon icon-inner_box"></i>'
        }
        menuHtml += '</div>';
        if (m.subMenu.type === _subType.list) {
            menuHtml += createSubMenuForList(m.subMenu.data);
        } else {
            menuHtml += m.subMenu.make();
        }
        menuHtml += '</li>';
    }
    menuHtml += '</ul>';
    menu.innerHTML = menuHtml;

    colorPadDemo = menu.querySelector('#' + _id.bgDemo);
    if (colorPadDemo) {
        colorPadDemo.style.color = '#fff';
    }

    return menu;
}

function createSubMenuForList(data) {
    var i, m,
        html = '<ul class="wiz-table-menu-sub">';
    for (i = 0; i < data.length; i++) {
        m = data[i];
        html += '<li class="wiz-table-menu-sub-item ' + _class.clickItem;
        if (m.isSplit) {
            html += ' split';
        }
        html += '" data-type="' + m.type + '">' + m.name;
        html += '</li>';
    }
    html += '</ul>';
    return html;
}

function getMenuTop() {
    var top, tableBody = menuObj.parentNode.querySelector('.' + CONST.CLASS.TABLE_BODY),
        tableBodyTop = tableBody ? tableBody.offsetTop : 0;
    top = tableBodyTop - menuObj.offsetHeight + 5;
    return top + 'px';
}
function fixMenuPos() {
    var container = menuObj.parentNode,
        offset = domUtils.getOffset(container),
        scrollTop = ENV.doc.body.scrollTop;

    if (scrollTop > offset.top - 30 && scrollTop < container.offsetHeight + offset.top - menuObj.offsetHeight * 2.5) {
        domUtils.css(menuObj, {
            position: 'fixed',
            top: '0',
            left: offset.left + 'px'
        });
    } else {
        domUtils.css(menuObj, {
            position: '',
            top: getMenuTop(),
            left: ''
        });
    }
}

var _event = {
    bind: function () {
        _event.unbind();
        if (menuObj) {
            menuObj.addEventListener('click', _event.handler.onClick);
            menuObj.addEventListener('mouseover', _event.handler.onMouseOver);
        }
        ENV.event.add(CONST.EVENT.BEFORE_SAVESNAP, _event.handler.onBeforeSaveSnap);
        ENV.event.add(CONST.EVENT.ON_SCROLL, _event.handler.onScroll);
    },
    unbind: function () {
        if (menuObj) {
            menuObj.removeEventListener('click', _event.handler.onClick);
            menuObj.removeEventListener('mouseover', _event.handler.onMouseOver);
        }
        ENV.event.remove(CONST.EVENT.BEFORE_SAVESNAP, _event.handler.onBeforeSaveSnap);
        ENV.event.remove(CONST.EVENT.ON_SCROLL, _event.handler.onScroll);
    },
    handler: {
        onBeforeSaveSnap: function () {
            // 目前不在保存快照前处理
            // tableMenu.hideSub();
        },
        onClick: function (e) {
            //点击 一级菜单
            var item = domUtils.getParentByFilter(e.target, function (dom) {
                return domUtils.hasClass(dom, CONST.CLASS.TABLE_MENU_BUTTON);
            }, true);
            if (item) {
                tableMenu.showSub(item.parentNode);
                return;
            }

            //点击 菜单具体功能
            var container;
            item = domUtils.getParentByFilter(e.target, function (dom) {
                return domUtils.hasClass(dom, _class.clickItem);
            }, true);
            if (!item || domUtils.hasClass(item, _class.disabled)) {
                return;
            }
            var type = item.getAttribute('data-type');
            var todo = true;
            switch (type) {
                case CONST.TYPE.TABLE.CLEAR_CELL:
                    tableCore.clearCellValue();
                    break;
                case CONST.TYPE.TABLE.MERGE_CELL:
                    tableCore.merge();
                    break;
                case CONST.TYPE.TABLE.SPLIT_CELL:
                    tableCore.split();
                    break;
                case CONST.TYPE.TABLE.INSERT_ROW_UP:
                    tableCore.insertRow(true);
                    break;
                case CONST.TYPE.TABLE.INSERT_ROW_DOWN:
                    tableCore.insertRow();
                    break;
                case CONST.TYPE.TABLE.INSERT_COL_LEFT:
                    tableCore.insertCol(true);
                    break;
                case CONST.TYPE.TABLE.INSERT_COL_RIGHT:
                    tableCore.insertCol();
                    break;
                case CONST.TYPE.TABLE.DELETE_ROW:
                    tableCore.deleteRows();
                    break;
                case CONST.TYPE.TABLE.DELETE_COL:
                    tableCore.deleteCols();
                    break;
                case CONST.TYPE.TABLE.SET_CELL_BG:
                    var bg = item.getAttribute('data-color');
                    tableCore.setCellBg(bg);
                    container = domUtils.getParentByFilter(item, function (dom) {
                        return domUtils.hasClass(dom, 'wiz-table-color-pad');
                    }, false);
                    domUtils.removeClass(container.querySelectorAll('.wiz-table-color-pad .' + _class.colorPadItem + '.' + _class.active), _class.active);
                    domUtils.addClass(item, _class.active);
                    colorPadDemo.setAttribute('data-last-color', bg);
                    break;
                case CONST.TYPE.TABLE.SET_CELL_ALIGN:
                    //设置 对齐方式 时，不自动隐藏二级菜单
                    var align = null, valign = null;
                    if (item.getAttribute('data-align-type') == 'align') {
                        align = item.getAttribute('data-align-value');
                    } else {
                        valign = item.getAttribute('data-align-value');
                    }
                    tableCore.setCellAlign(align, valign);

                    container = item.parentNode;
                    domUtils.removeClass(container.querySelectorAll('.' + _class.active), _class.active);
                    domUtils.addClass(item, _class.active);
                    todo = false;
                    break;
                case CONST.TYPE.TABLE.DELETE_TABLE:
                    tableCore.deleteTable();
                    break;
                case CONST.TYPE.TABLE.DISTRIBUTE_COLS:
                    tableCore.distributeCols();
                    break;
                default:
                    todo = false;
            }

            if (todo) {
                tableMenu.hideSub();
            }

        },
        onMouseOver: function (e) {
            var colorItem = domUtils.getParentByFilter(e.target, function (dom) {
                return domUtils.hasClass(dom, _class.colorPadItem);
            }, true);
            if (colorItem && colorPadDemo) {
                colorPadDemo.style.color = colorItem.getAttribute('data-color') || '#fff';
            }
        },
        onScroll: function (e) {
            if (!menuObj || menuObj.style.display == 'none') {
                return;
            }
            fixMenuPos();
        }
    }
};

var tableMenu = {
    init: function (_tableCore) {
        tableCore = _tableCore;
    },
    hide: function () {
        if (menuObj) {
            menuObj.style.display = 'none';
        }
        _event.unbind();
    },
    hideSub: function () {
        if (!menuObj) {
            return;
        }
        var sub = menuObj.querySelectorAll('.' + CONST.CLASS.TABLE_MENU_ITEM + '.' + _class.active);
        domUtils.removeClass(sub, _class.active);

        if (colorPadDemo) {
            colorPadDemo.style.color = colorPadDemo.getAttribute('data-last-color') || '#fff';
        }
    },
    remove: function () {
        if (menuObj) {
            domUtils.remove(menuObj);
            menuObj = null;
        }
    },
    show: function () {
        if (ENV.client.type.isPhone || ENV.client.type.isPad) {
            return;
        }
        var zone = tableZone.getZone();
        if (!zone.grid || !zone.range) {
            tableMenu.hide();
            return;
        }

        var container = domUtils.getParentByFilter(zone.table, function (dom) {
            return domUtils.hasClass(dom, CONST.CLASS.TABLE_CONTAINER);
        }, false);
        menuObj = createMenu();
        domUtils.attr(menuObj, {
            contenteditable: 'false'
        });
        tableMenu.hideSub();
        container.appendChild(menuObj);
        domUtils.css(menuObj, {
            top: getMenuTop()
        });
        menuObj.style.display = 'block';

        fixMenuPos();
        _event.bind();
    },
    showSub: function (item) {
        if (domUtils.hasClass(item, _class.active)) {
            domUtils.removeClass(item, _class.active);
            return;
        }

        //控制二级菜单 默认值
        var canMerge, canSplit, cellAlign,
            subItem, zone = tableZone.getZone();
        if (item.id === _id.cells) {
            canMerge = tableUtils.canMerge(zone.grid, zone.range);
            canSplit = tableUtils.canSplit(zone.grid, zone.range);

            subItem = item.querySelector('[data-type=' + CONST.TYPE.TABLE.MERGE_CELL + ']');
            if (subItem && canMerge) {
                domUtils.removeClass(subItem, _class.disabled);
            } else if (subItem) {
                domUtils.addClass(subItem, _class.disabled);
            }

            subItem = item.querySelector('[data-type=' + CONST.TYPE.TABLE.SPLIT_CELL + ']');
            if (subItem && canSplit) {
                domUtils.removeClass(subItem, _class.disabled);
            } else if (subItem) {
                domUtils.addClass(subItem, _class.disabled);
            }
        } else if (item.id === _id.align) {
            cellAlign = tableUtils.getAlign(zone.grid, zone.range);
            subItem = item.querySelector('.' + _class.alignItem + '.' + _class.active + '[data-align-type=align]');
            if (subItem && (!cellAlign.align || subItem.getAttribute('data-align-value').toLowerCase() !== cellAlign.align)) {
                domUtils.removeClass(subItem, _class.active);
                subItem = null;
            }
            if (!subItem && cellAlign.align) {
                subItem = item.querySelector('[data-align-value=' + cellAlign.align + ']');
                domUtils.addClass(subItem, _class.active);
            }

            subItem = item.querySelector('.' + _class.alignItem + '.' + _class.active + '[data-align-type=valign]');
            if (subItem && (!cellAlign.valign || subItem.getAttribute('data-align-value').toLowerCase() !== cellAlign.valign)) {
                domUtils.removeClass(subItem, _class.active);
                subItem = null;
            }
            if (!subItem && cellAlign.valign) {
                subItem = item.querySelector('[data-align-value=' + cellAlign.valign + ']');
                domUtils.addClass(subItem, _class.active);
            }
        }

        tableMenu.hideSub();
        domUtils.addClass(item, _class.active);
    }
};

module.exports = tableMenu;
},{"../common/const":18,"../common/env":20,"../common/lang":22,"../domUtils/domExtend":29,"./tableUtils":54,"./tableZone":55}],54:[function(require,module,exports){
/**
 * 表格操作的基本方法集合
 */
var ENV = require('./../common/env'),
    CONST = require('./../common/const'),
    utils = require('./../common/utils'),
    domUtils = require('./../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend');

/**
 * table 相关的 默认值
 *
 */

var tableUtils = {
    /**
     * 初始化 默认值
     * @param options
     */
    init: function () {
    },
    /**
     * 判断当前是否允许新建表格
     * @param zone
     * @returns {boolean}
     */
    canCreateTable: function (zone) {
        var range = rangeUtils.getRange(), tmpCell;
        if (range) {
            tmpCell = domUtils.getParentByTagName(range.startContainer, ['table'], true, null) ||
                domUtils.getParentByTagName(range.endContainer, ['table'], true, null);
            if (tmpCell) {
                return false;
            }
            if (domUtils.getParentByClass(range.startContainer, CONST.CLASS.CODE_CONTAINER, true)) {
                // CodeMirror 内
                return false;
            }
        }
        return !zone.range;
    },
    /**
     * 判断选择的单元格是否能进行 合并操作
     * @param grid
     * @param range
     * @returns {*|boolean}
     */
    canMerge: function (grid, range) {
        return (grid && range &&
        grid[range.minY][range.minX].cell !== grid[range.maxY][range.maxX].cell);
    },
    /**
     * 判断选择的单元格是否能被拆分
     * @param grid
     * @param range
     * @returns {*}
     */
    canSplit: function (grid, range) {
        if (!grid || !range) {
            return false;
        }
        var key;
        var splitMap = {}, canSplit = false;
        tableUtils.eachRange(grid, range, function (cellData) {
            key = cellData.x_src + '_' + cellData.y_src;
            if (cellData.fake && !splitMap[key]) {
                splitMap[key] = grid[cellData.y_src][cellData.x_src];
                canSplit = true;
            }
        });
        return canSplit ? splitMap : false;
    },
    /**
     * 检查 光标是否处于 table & tableContainer 之间
     * @param e
     * @returns {{tableContainer: null, before: boolean, after: boolean}}
     */
    checkCaretInTableContainer: function (e) {
        var result = {
            tableContainer: null,
            tableMenu: false,
            before: false,
            after: false
        };
        var range, tableContainer, table,
            target, startOffset, start;
        var eType = /^(mouse|touch)/i;

        if (e && eType.test(e.type)) {
            //mouse || touch 事件触发时， selection 并没有被改变，所以不能使用 range 进行判断
            target = e.target;

        } else {
            range = rangeUtils.getRange();
            if (!range || !range.collapsed) {
                //选择区域 由 keyUp & mouseUp 中的 tableUtils.fixSelection(e); 进行过滤
                return result;
            }

            start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
            target = start.container;
            startOffset = start.offset;
        }

        if (domUtils.isTag(target, 'table')) {
            tableUtils.initTableContainer(target);
        }

        if (tableUtils.isMenu(target)) {
            result.tableContainer = tableUtils.getContainerExcludeTable(target);
            result.tableMenu = true;
            return result;
        }

        tableContainer = tableUtils.getContainerExcludeTable(target);

        if (tableContainer) {
            result.tableContainer = tableContainer;
            if (e && eType.test(e.type)) {
                table = tableContainer.querySelector('table');
            } else if (startOffset > 0) {
                result.after = true;
            } else {
                result.before = true;
            }
        }
        return result;
    },
    /**
     * 清空选中区域单元格内的数据
     * @param grid
     * @param range
     */
    clearCellValue: function (grid, range) {
        if (!grid || !range) {
            return;
        }
        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                cellData.cell.innerHTML = '<br/>';
            }
        });
    },
    /**
     * 复制单元格 Dom
     * @param cell
     * @param isClear
     * @returns {Element}
     */
    cloneCell: function (cell, isClear) {
        var newCell = ENV.doc.createElement(cell.tagName);
        newCell.style.cssText = cell.style.cssText;
        if (isClear) {
            newCell.innerHTML = '<br/>';
        } else {
            newCell.colSpan = cell.colSpan;
            newCell.rowSpan = cell.rowSpan;
            newCell.innerHTML = cell.innerHTML;
        }
        // TODO 处理 已选中的 cell
        return newCell;
    },
    /**
     * 创建 单元格
     * @param width
     * @returns {Element}
     */
    createCell: function (width) {
        var td = ENV.doc.createElement('td');
        td.setAttribute('align', 'left');
        td.setAttribute('valign', 'middle');
        if (width) {
            td.setAttribute('style', 'width:' + width + 'px');
        }
        td.appendChild(ENV.doc.createElement('br'));
        return td;
    },
    /**
     * 创建 表格
     * @param col
     * @param row
     * @returns {Element}
     */
    createTable: function (col, row) {
        if (!col || !row) {
            return;
        }

        var table = ENV.doc.createElement('table'),
            tbody = ENV.doc.createElement('tbody'),
            tr, c, r;

        for (r = 0; r < row; r++) {
            tr = ENV.doc.createElement('tr');
            for (c = 0; c < col; c++) {
                tr.appendChild(tableUtils.createCell(ENV.options.table.colWidth));
            }
            tbody.appendChild(tr);
        }

        table.appendChild(tbody);
        table.style.width = (ENV.options.table.colWidth * col) + 'px';
        return table;
    },
    /**
     * 删除指定的列
     * @param grid
     * @param col
     */
    deleteCols: function (grid, col) {
        if (!grid || grid.length === 0 || col > grid[0].length) {
            return;
        }
        var table = domUtils.getParentByTagName(grid[0][0].cell, 'table', false, null);

        var tmpCellList = [], width = ENV.options.table.colWidth;

        var y, g, cell;
        for (y = 0; y < grid.length; y++) {
            g = grid[y][col];
            if (g.y_src == y && g.cell.colSpan > 1) {
                g.cell.colSpan--;
                tmpCellList.push(g.cell);
            } else if (g.y_src == y) {
                width = tableUtils.getCellWidth(g.cell);
                g.cell.parentElement.removeChild(g.cell);
            }
            grid[y].splice(col, 1);
        }

        for (y = 0; y < tmpCellList.length; y++) {
            cell = tmpCellList[y];
            cell.style.width = (tableUtils.getCellWidth(cell) - width) + 'px';
        }

        //如果所有单元格都删除了，则删除表格
        if (!table.getElementsByTagName('td').length && !table.getElementsByTagName('th').length) {
            table.parentElement.removeChild(table);
        } else {
            tableUtils.fixTableWidth(table);
        }

    },
    /**
     * 删除指定的行
     * @param grid
     * @param row
     */
    deleteRows: function (grid, row) {
        if (!grid || grid.length === 0 || row > grid.length) {
            return;
        }

        var table = domUtils.getParentByTagName(grid[0][0].cell, 'table', false, null),
            rows = table.rows;

        var x, g, cellData;
        for (x = grid[row].length - 1; x >= 0; x--) {
            g = grid[row][x];
            if (g.x_src == x && g.y_src < g.y) {
                g.cell.rowSpan--;
            } else if (g.x_src == x && g.y_src == g.y && g.cell.rowSpan > 1 &&
                (row + 1) < grid.length) {
                //row+1 防止表格异常的 rowSpan 设置

                g.cell.rowSpan--;
                cellData = tableUtils.getNextCellDataInRow(grid[row + 1], x);
                cellData = cellData ? cellData.cell : null;
                rows[row + 1].insertBefore(g.cell, cellData);
            }
        }
        grid.splice(row, 1);
        rows[row].parentElement.removeChild(rows[row]);

        //如果所有单元格都删除了，则删除表格
        if (!table.getElementsByTagName('tr').length) {
            table.parentElement.removeChild(table);
        } else {
            tableUtils.fixTableWidth(table);
        }

    },
    /**
     * 平均分配每列
     * @param table
     * @param grid
     */
    distributeCols: function (table, grid) {
        if (!table || !grid) {
            return;
        }
        var colCount = grid[0].length;
        if (colCount === 0) {
            return;
        }

        var rows = table.rows,
            w = table.offsetWidth / colCount,
            y, x, cell;

        for (y = rows.length - 1; y >= 0; y--) {
            for (x = rows[y].cells.length - 1; x >= 0; x--) {
                cell = rows[y].cells[x];
                cell.style.width = w * cell.colSpan + 'px';
            }
        }
        table.style.width = table.offsetWidth + 'px';
    },
    /**
     * each 循环遍历 选中区域的单元格
     * @param grid
     * @param range
     * @param callback
     */
    eachRange: function (grid, range, callback) {
        if (!grid || !range || !callback || typeof callback !== 'function') {
            return;
        }

        var x, y, cbBreak = true;
        for (y = range.minY; cbBreak !== false && y < grid.length && y <= range.maxY; y++) {
            for (x = range.minX; cbBreak !== false && x < grid[y].length && x <= range.maxX; x++) {
                cbBreak = callback(grid[y][x]);
            }
        }
    },
    /**
     * 修正选中区域的 selection
     */
    fixSelection: function () {
        //避免选择文本时， 选中到 表格内部
        var range = rangeUtils.getRange();
        if (!range || range.collapsed) {
            return;
        }

        var start = range.startContainer,
            startOffset = range.startOffset,
            end = range.endContainer,
            endOffset = range.endOffset,
            startTr = tableUtils.getContainer(start),
            endTr = tableUtils.getContainer(end);
        if ((!startTr && !endTr) || (startTr && endTr)) {
            return;
        }

        var table,
            target = startTr ? startTr : endTr;

        while (table = tableUtils.getContainer(target)) {
            if (startTr) {
                target = domUtils.getNextNode(table, false, end);
            } else {
                target = domUtils.getPreviousNode(table, false, start);
            }
        }

        if (startTr) {
            start = target ? target : end;
            startOffset = 0;
        } else {
            end = target ? target : start;
            endOffset = domUtils.getEndOffset(end);
        }

        if (startTr) {
            rangeUtils.setRange(end, endOffset, start, startOffset);
        } else {
            rangeUtils.setRange(start, startOffset, end, endOffset);
        }
    },
    /**
     * 修正 table 宽度
     * @param table
     */
    fixTableWidth: function (table) {
        if (!table) {
            return;
        }
        var rows = table.rows, i, cell, w, tableWidth = 0;
        for (i = 0; i < rows[0].cells.length; i++) {
            cell = rows[0].cells[i];
            w = tableUtils.getCellWidth(cell);
            tableWidth += w;
        }
        table.style.width = tableWidth + 'px';
    },
    /**
     * 获取 选中区域内综合的单元格对齐方式
     * @param grid
     * @param range
     * @returns {*}
     */
    getAlign: function (grid, range) {
        if (!grid || !range) {
            return false;
        }
        var align, valign, cell,
            result = {
                align: '',
                valign: ''
            };
        tableUtils.eachRange(grid, range, function (cellData) {
            cell = cellData.cell;
            if (!cellData.fake) {
                align = cell.align.toLowerCase();
                valign = cell.vAlign.toLowerCase();
            }

            if (result.align === '') {
                result.align = align;
                result.valign = valign;
            }

            if (result.align !== null) {
                result.align = (result.align === align) ? align : null;
            }
            if (result.valign !== null) {
                result.valign = (result.valign === valign) ? valign : null;
            }

            return result.align !== null || result.valign !== null;
        });

        return result;
    },
    /**
     * 获取单元格宽度
     * @param cell
     * @returns {Number}
     */
    getCellWidth: function (cell) {
        return parseInt(cell.style.width || cell.offsetWidth, 10);
    },
    /**
     * 根据 单元格 dom 获取 grid 内对应的 data 数据
     * @param grid
     * @param cell
     * @returns {*}
     */
    getCellData: function (grid, cell) {
        if (!grid || !cell) {
            return null;
        }
        var i, j, g;
        for (i = 0; i < grid.length; i++) {
            for (j = 0; j < grid[i].length; j++) {
                g = grid[i][j];
                if (g.cell === cell) {
                    return g;
                }
            }
        }
        return null;
    },
    /**
     * 根据 rang 范围 获取 单元格的 data 列表
     * @param grid
     * @param range
     * @returns {Array}
     */
    getCellsByRange: function (grid, range) {
        var cellList = [];
        if (!grid || !range) {
            return cellList;
        }
        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                cellList.push(cellData.cell);
            }
        });
        return cellList;
    },
    getContainer: function (target) {
        return domUtils.getParentByClass(target, CONST.CLASS.TABLE_CONTAINER, true);
    },
    getTableBody: function (target) {
        return domUtils.getParentByFilter(target, function (dom) {
            return domUtils.hasClass(dom, CONST.CLASS.TABLE_BODY);
        }, true)
    },
    /**
     * 根据 target 目标 dom 判断是否处于 table 容器内且 在 table 之外
     * @param target
     * @returns {null}
     */
    getContainerExcludeTable: function (target) {
        var cell = domUtils.getParentByTagName(target, ['th', 'td'], true, null);
        var tableContainer = !cell ? tableUtils.getContainer(target) : null;
        return tableContainer;
    },
    /**
     * 从 cell 集合中遍历获取 cell 内的叶子节点集合
     * @param cellList
     * @returns {Array}
     */
    getDomsByCellList: function (cellList) {
        var i, j, cell, tmpList, domList = [];
        if (!cellList) {
            return domList;
        }
        for (i = 0, j = cellList.length; i < j; i++) {
            cell = cellList[i];
            tmpList = domUtils.getListA2B({
                startDom: cell.firstChild,
                startOffset: 0,
                endDom: cell.lastChild,
                endOffset: 1,
                noSplit: true
            });
            domList = domList.concat(tmpList.list);
        }
        return domList;
    },
    /**
     * 在表格内根据指定单元格 获取 下一个 单元格，到达最后一列时，自动从下一行查找
     * @param cell
     * @returns {*}
     */
    getNextCellInTable: function (cell) {
        var nextCell = cell.nextElementSibling;
        if (nextCell) {
            return nextCell;
        }
        var tr = cell.parentNode.nextElementSibling;
        while (tr) {
            if (tr.cells.length > 0) {
                return tr.cells[0];
            }
            tr = tr.nextElementSibling;
        }
        return null;
    },
    /**
     * 在一行内 根据列数获取下一个单元格
     * @param gridRow
     * @param col
     * @returns {*}
     */
    getNextCellDataInRow: function (gridRow, col) {
        if (!gridRow) {
            return null;
        }
        var i;
        for (i = col; i < gridRow.length; i++) {
            if (!gridRow[i].fake) {
                return gridRow[i];
            }
        }
        return null;
    },
    /**
     * 根据 mouse 相关事件，获取 对应的 坐标位置
     * @param e
     * @param table
     * @returns {{clientX: *, clientY: *}}
     */
    getMousePosition: function (e, table) {
        var eventClient = utils.getEventClientPos(e);
        var tableBody = tableUtils.getTableBody(table || e.target);
        var clientX = eventClient.x + ENV.doc.body.scrollLeft + (tableBody ? tableBody.scrollLeft : 0);
        var clientY = eventClient.y + ENV.doc.body.scrollTop + (tableBody ? tableBody.scrollTop : 0);
        return {
            x: clientX,
            y: clientY
        };
    },
    /**
     * 在一行内 根据列数获取上一个单元格
     * @param gridRow
     * @param col
     * @returns {*}
     */
    getPreviousCellDataInRow: function (gridRow, col) {
        if (!gridRow) {
            return null;
        }
        var i;
        for (i = col; i >= 0; i--) {
            if (!gridRow[i].fake) {
                return gridRow[i];
            }
        }
        return null;
    },
    /**
     * 根据 节点的 cellData 获取单元格所占面积
     * @param cellData
     * @returns {*}
     */
    getRangeByCellData: function (cellData) {
        if (!cellData) {
            return {
                minX: 0,
                minY: 0,
                maxX: 0,
                maxY: 0
            };
        }
        return {
            minX: cellData.x_src,
            minY: cellData.y_src,
            maxX: cellData.x_src + cellData.cell.colSpan - 1,
            maxY: cellData.y_src + cellData.cell.rowSpan - 1
        };
    },
    /**
     * 根据起始单元格的 data 数据 获取 grid 中的 range
     * @param grid
     * @param startData
     * @param endData
     * @returns {*}
     */
    getRangeByCellsData: function (grid, startData, endData) {
        if (!grid || !startData || !endData) {
            return null;
        }

        var startRange = tableUtils.getRangeByCellData(startData);
        if (startData.cell === endData.cell) {
            return startRange;
        }
        var endRange = tableUtils.getRangeByCellData(endData);

        var minX = Math.min(startRange.minX, endRange.minX),
            minY = Math.min(startRange.minY, endRange.minY),
            maxX = Math.max(startRange.maxX, endRange.maxX),
            maxY = Math.max(startRange.maxY, endRange.maxY),
            _minX, _minY, _maxX, _maxY;

        var x, y, g, gRange, k, cellMap = {}, changeRange = true;

        // console.log(minX + ',' + minY + ' - ' + maxX + ',' + maxY);
        while (changeRange) {
            changeRange = false;
            _minX = minX;
            _minY = minY;
            _maxX = maxX;
            _maxY = maxY;
            for (y = minY; y <= maxY; y++) {
                for (x = minX; x <= maxX; x++) {
                    // console.log('['+x+','+y+']' +minX + ',' + minY + ' - ' + maxX + ',' + maxY);
                    //遍历范围时，只需要寻找边缘的 Cell 即可
                    if (y > minY && y < maxY && x < maxX - 1) {
                        x = maxX - 1;
                        continue;
                    }

                    g = grid[y][x];
                    k = g.x_src + '_' + g.y_src;
                    if (cellMap[k]) {
                        //如果该 Cell 已经被计算过，则不需要重新计算
                        continue;
                    }

                    gRange = tableUtils.getRangeByCellData(g);
                    minX = Math.min(minX, gRange.minX);
                    minY = Math.min(minY, gRange.minY);
                    maxX = Math.max(maxX, gRange.maxX);
                    maxY = Math.max(maxY, gRange.maxY);

                    if (minX !== _minX || minY !== _minY ||
                        maxX !== _maxX || maxY !== _maxY) {
                        changeRange = true;
                        break;
                    }
                }
                if (changeRange) {
                    break;
                }
            }
        }

        return {
            minX: minX,
            minY: minY,
            maxX: maxX,
            maxY: maxY
        }
    },
    /**
     * 根据 表格 获取 grid
     * @param table
     * @returns {*}
     */
    getTableGrid: function (table) {
        if (!table || !domUtils.isTag(table, 'table')) {
            return null;
        }
        var grid = [];
        var c, r, rows, row, cells, cell,
            colSpan, rowSpan, i, j, x, y, x_src, y_src,
            startX;

        rows = table.rows;
        for (r = 0; r < rows.length; r++) {
            row = rows[r];
            cells = row.cells;

            if (!grid[r]) {
                grid[r] = [];
            }
            for (c = 0; c < cells.length; c++) {
                cell = cells[c];
                colSpan = cell.colSpan;
                rowSpan = cell.rowSpan;

                // 从 excel 复制并粘贴的 table 可能 td 内为空
                if (!cell.firstChild) {
                    cell.innerHTML = '<br>';
                }

                startX = getX(c, r);
                for (i = 0; i < rowSpan; i++) {
                    if (!grid[r + i]) {
                        grid[r + i] = [];
                    }
                    for (j = 0; j < colSpan; j++) {
                        y = r + i;
                        x = getX(startX + j, y);
                        if (i == 0 && j == 0) {
                            x_src = x;
                            y_src = y;
                        }
                        grid[y][x] = {
                            cell: cell,
                            x: x,
                            y: y,
                            x_src: x_src,
                            y_src: y_src,
                            fake: i > 0 || j > 0
                        }
                    }
                }
            }
        }

        return grid;

        function getX (index, y) {
            while (grid[y][index]) {
                index++;
            }
            return index;
        }
    },
    /**
     * 分析 并处理 剪切板内得到的 html 代码
     * @param html
     * @returns {{isTable: boolean, pasteDom: *}}
     */
    getTemplateByHtmlForPaste: function (html) {
        var pasteTables, pasteTable,
            pasteIsTable = false, pasteDom,
            i, j,
            template = ENV.doc.createElement('div');

        template.innerHTML = html;
        //清理无效dom
        domUtils.childNodesFilter(template);

        pasteTables = template.querySelectorAll('table');
        if (pasteTables.length == 1) {
            pasteTable = pasteTables[0];
            domUtils.remove(pasteTable);
            if (domUtils.isEmptyDom(template)) {
                pasteIsTable = true;
                pasteDom = pasteTable;
            } else {
                //如果不是单一表格，恢复 innerHTML 便于后面标准处理
                template.innerHTML = html;
            }
        }

        if (!pasteIsTable) {
            pasteTables = template.querySelectorAll('table');
            //表格内 禁止粘贴表格，所以需要把表格全部变为 text
            for (i = pasteTables.length - 1; i >= 0; i--) {
                pasteTable = pasteTables[i];
                domUtils.before(ENV.doc.createTextNode(pasteTable.innerText), pasteTable);
                domUtils.remove(pasteTable);
            }
            //清理 template 内的多余 nodeType
            for (i = template.childNodes.length - 1; i >= 0; i--) {
                j = template.childNodes[i];
                if (j.nodeType !== 1 && j.nodeType !== 3 && domUtils.isEmptyDom(j)) {
                    template.removeChild(j);
                }
            }
            pasteDom = template;
        }
        return {
            isTable: pasteIsTable,
            pasteDom: pasteDom
        }
    },
    /**
     * 分析 并处理 剪切板内得到的 text 代码
     * @param txt
     * @returns {{isTable: boolean, pasteDom: Element}}
     */
    getTemplateByTxtForPaste: function (txt) {
        txt = (txt || '').trim();
        var rows = txt.split(/\r?\n/),
            x, y, cols,
            table = ENV.doc.createElement('table'),
            tbody = ENV.doc.createElement('tbody'),
            tr, td, maxX = 0;

        table.appendChild(tbody);
        for (y = 0; y < rows.length; y++) {
            cols = rows[y].split('\u0009');
            tr = ENV.doc.createElement('tr');
            for (x = 0; x < cols.length; x++) {
                td = tableUtils.createCell();
                if (cols[x]) {
                    td.innerHTML = '';
                    td.appendChild(ENV.doc.createTextNode(cols[x]));
                }
                tr.appendChild(td);
            }
            maxX = Math.max(maxX, tr.cells.length);
            tbody.appendChild(tr);
        }

        //避免 table 列数不一致
        rows = table.rows;
        for (y = 0; y < rows.length; y++) {
            tr = rows[y];
            cols = tr.cells;
            for (x = cols.length; x < maxX; x++) {
                tr.appendChild(tableUtils.createCell());
            }
        }

        return {
            isTable: true,
            pasteDom: table
        }
    },
    /**
     * 初始化 表格 样式
     * @param table
     */
    initTable: function (table) {
        var i, j, cell;
        for (i = table.rows.length - 1; i >= 0; i--) {
            for (j = table.rows[i].cells.length - 1; j >= 0; j--) {
                cell = table.rows[i].cells[j];
                cell.style.width = cell.offsetWidth + 'px';
            }
        }
        table.style.width = table.offsetWidth + 'px';
    },
    /**
     * 检查 并 初始化表格容器
     * @param _table
     */
    initTableContainer: function (_table) {
        var tableList = _table ? [_table] : ENV.doc.querySelectorAll('table'),
            table, container, tableBody, i, j;

        for (i = 0, j = tableList.length; i < j; i++) {
            table = tableList[i];
            tableBody = checkParent(table, function (parent) {
                if (parent === ENV.doc.body) {
                    return false;
                }
                if (parent.childNodes.length === 1) {
                    domUtils.addClass(parent, CONST.CLASS.TABLE_BODY);
                    return true;
                }
                return domUtils.hasClass(parent, CONST.CLASS.TABLE_BODY);
            });

            container = checkParent(tableBody, function (parent) {
                if (parent === ENV.doc.body) {
                    return false;
                }
                if (parent.childNodes.length === 1) {
                    domUtils.addClass(parent, CONST.CLASS.TABLE_CONTAINER);
                    return true;
                }
                return domUtils.hasClass(parent, CONST.CLASS.TABLE_CONTAINER);
            });

            domUtils.addClass(container, CONST.CLASS.TABLE_CONTAINER);
            //避免 编辑、阅读状态切换时， 表格位置闪动，所以做成 inline 模式
            domUtils.css(container, {
                position: 'relative',
                padding: '0'
            });
            domUtils.addClass(tableBody, CONST.CLASS.TABLE_BODY);
            domUtils.removeClass(tableBody, CONST.CLASS.TABLE_MOVING);
        }

        function checkParent (obj, filter) {
            var parent = obj.parentNode;
            if (!filter(parent)) {
                parent = ENV.doc.createElement('div');
                domUtils.before(parent, obj);
                parent.appendChild(obj);
            }
            return parent;
        }
    },
    /**
     * 在指定的位置插入列
     * @param grid
     * @param col
     */
    insertCol: function (grid, col) {
        if (!grid) {
            return;
        }
        col = col || 0;
        var y, gRow, g, cell, newCell, nextCellData;
        var table = domUtils.getParentByTagName(grid[0][0].cell, 'table', false, null),
            rows = table.rows, lastCell = null;
        for (y = 0; y < grid.length; y++) {
            gRow = grid[y];

            if (gRow.length > col) {
                g = grid[y][col];
                cell = g.cell;
            } else {
                g = null;
                cell = null;
            }

            if (cell && cell !== lastCell && g.x_src < col) {
                //cell.colSpan > 1
                g.cell.colSpan++;

                // 需要调整 style
                g.cell.style.width = (tableUtils.getCellWidth(g.cell) + ENV.options.table.colWidth) + 'px';

            } else if (!cell || (cell && g.x_src == col)) {

                newCell = tableUtils.createCell(ENV.options.table.colWidth);
                if (cell && g.y_src < g.y) {
                    //cell.rowSpan > 1
                    nextCellData = tableUtils.getNextCellDataInRow(grid[y], col);
                    rows[y].insertBefore(newCell, (nextCellData ? nextCellData.cell : null));
                } else {
                    rows[y].insertBefore(newCell, cell);
                }
            }
            lastCell = g ? g.cell : null;
        }

        tableUtils.fixTableWidth(table);
    },
    /**
     * 在指定的位置插入行
     * @param grid
     * @param row
     */
    insertRow: function (grid, row) {
        if (!grid) {
            return;
        }
        row = row || 0;
        var x, g, newCell;
        var table = domUtils.getParentByTagName(grid[0][0].cell, 'table', false, null),
            tr = ENV.doc.createElement('tr');
        var gRow = grid[grid.length > row ? row : grid.length - 1];
        for (x = 0; x < gRow.length; x++) {
            g = gRow[x];

            if (grid.length > row && g.y_src < g.y && g.x_src == g.x) {
                //cell.rowSpan > 1
                g.cell.rowSpan++;
                // TODO 需要调整 style( height)

            } else if (grid.length <= row || (g.y_src == g.y)) {
                newCell = tableUtils.cloneCell(g.cell, true);
                if (g.cell.colSpan > 1) {
                    newCell.style.width = g.cell.offsetWidth / g.cell.colSpan + 'px';
                }
                tr.appendChild(newCell);
            }
        }

        var target = gRow[0].cell.parentElement,
            parent = target.parentElement;
        if (grid.length <= row) {
            target = null;
        }
        parent.insertBefore(tr, target);
    },
    isMenu: function (dom) {
        if (!dom) {
            return false;
        }
        return !!domUtils.getParentByFilter(dom, function (p) {
            return domUtils.hasClass(p, CONST.CLASS.TABLE_TOOLS);
        }, true);
    },
    /**
     * 将指定的单元格范围进行合并
     * @param grid
     * @param range
     * @returns {*}
     */
    mergeCell: function (grid, range) {
        if (!tableUtils.canMerge(grid, range)) {
            return null;
        }

        var dy = range.maxY - range.minY + 1,
            dx = range.maxX - range.minX + 1;

        var target = grid[range.minY][range.minX].cell;
        var temp = ENV.doc.createElement('div');
        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake && cellData.cell != target) {
                if (!domUtils.isEmptyDom(cellData.cell)) {
                    if (temp.lastChild) {
                        temp.appendChild(ENV.doc.createElement('br'));
                    }
                    while (cellData.cell.firstChild) {
                        temp.appendChild(cellData.cell.firstChild);
                    }
                }
                domUtils.remove(cellData.cell);
            }
        });
        while (temp.firstChild) {
            target.appendChild(temp.firstChild);
        }
        target.rowSpan = dy;
        target.colSpan = dx;
        return target;
    },
    /**
     * 修改单元格 Dom 样式 & 属性
     * @param style
     * @param attr
     */
    modifySelectionDom: function (zone, style, attr) {
        var align, valign;

        //处理单元格样式
        align = style['text-align'] || null;
        valign = style['text-valign'] || null;
        delete style['text-align'];
        delete style['text-valign'];

        tableUtils.eachRange(zone.grid, zone.range, function (cellData) {
            if (!cellData.fake) {
                rangeUtils.modifyDomsStyle(cellData.cell.childNodes, style, attr, []);
            }
        });
        if (align || valign) {
            tableUtils.setCellAlign(zone.grid, zone.range, {
                align: align,
                valign: valign
            });
        }
    },
    /**
     * 设置单元格对齐方式
     * @param grid
     * @param range
     * @param _alignType
     */
    setCellAlign: function (grid, range, _alignType) {
        if (!grid || !range) {
            return;
        }

        var alignType = {};
        if (_alignType.align != null) {
            alignType.align = _alignType.align || 'left';
        }
        if (_alignType.valign != null) {
            alignType.valign = _alignType.valign || 'middle';
        }

        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                if (alignType.align) {
                    domUtils.css(cellData.cell, {'text-align': ''});
                }
                if (alignType.valign) {
                    domUtils.css(cellData.cell, {'text-valign': ''});
                }
                domUtils.attr(cellData.cell, alignType);
            }
        });
    },
    /**
     * 设置单元格背景颜色
     * @param grid
     * @param range
     * @param bgColor
     */
    setCellBg: function (grid, range, bgColor) {
        if (!grid || !range) {
            return;
        }

        bgColor = bgColor || '';
        if (bgColor.toLowerCase() === 'transparent') {
            bgColor = '';
        }

        tableUtils.eachRange(grid, range, function (cellData) {
            if (!cellData.fake) {
                domUtils.css(cellData.cell, {
                    'background-color': bgColor
                })
            }
        });
    },
    getKeyForColWidth: function (g) {
        return g.x_src + '_' + g.y_src;
    },
    fixColTimer: -1,
    fixColWidth: function (table, tableWidth, cells, cellMap, dx) {
        if (dx >= 0) {
            return;
        }
        // 表格其特殊性，导致 单元格宽度修改后，必须要进行校验，
        // 因为有些在 单元格内的内容是不允许改变 width 的，导致单元格也无法 缩小 width
        if (tableUtils.fixColTimer) {
            clearTimeout(tableUtils.fixColTimer);
        }
        tableUtils.fixColTimer = setTimeout(function () {
            var i, j, g, tmpDx, maxDx = dx;
            table.style.width = tableWidth + 'px';
            for (i = 0, j = cells.length; i < j; i++) {
                g = cells[i];
                tmpDx = g.cell.offsetWidth - cellMap[tableUtils.getKeyForColWidth(g)] + dx;
                // dx < 0 所以找到变化最小的 单元格应该 是 max
                maxDx = i === 0 ? tmpDx : Math.max(maxDx, tmpDx);
            }

            if (maxDx > dx) {
                // console.log('maxDx = ' + maxDx);
                table.style.width = tableWidth - dx + maxDx + 'px';
                // 缩减 width 时，修正 width
                for (i = 0, j = cells.length; i < j; i++) {
                    g = cells[i];
                    g.cell.style.width = cellMap[tableUtils.getKeyForColWidth(g)] - dx + maxDx + 'px';
                    // console.log(cellMap[tableUtils.getKeyForColWidth(g)] + ' -> ' + (cellMap[tableUtils.getKeyForColWidth(g)] - dx + maxDx));
                }
                ENV.event.call(CONST.EVENT.UPDATE_RENDER);
            }
        }, 0);
    },
    /**
     * 设置列宽
     * @param table
     * @param grid
     * @param col
     * @param dx
     */
    setColWidth: function (table, grid, col, dx) {
        dx = fixDx();
        var tableWidth = table.offsetWidth + dx;
        var i, j, g, key,
            cells = [], cellMap = {};
        for (i = 0, j = grid.length; i < j; i++) {
            g = grid[i][col];
            key = tableUtils.getKeyForColWidth(g);
            if (!cellMap[key]) {
                cellMap[key] = g.cell.offsetWidth + dx;
                cells.push(g);
            }
        }

        table.style.width = tableWidth + 'px';
        for (i = 0, j = cells.length; i < j; i++) {
            g = cells[i];
            g.cell.style.width = cellMap[tableUtils.getKeyForColWidth(g)] + 'px';
        }

        tableUtils.fixColWidth(table, tableWidth, cells, cellMap, dx);

        function fixDx () {
            var y, g, cell, maxDx = dx, tmpDx;
            for (y = 0; y < grid.length; y++) {
                g = grid[y][col];
                tmpDx = ENV.options.table.colWidthMin - g.cell.offsetWidth;
                if (g.cell.colSpan == 1) {
                    maxDx = tmpDx;
                    cell = g.cell;
                    break;
                }
                if (maxDx < tmpDx) {
                    maxDx = tmpDx;
                    cell = g.cell;
                }
            }

            if (dx < maxDx) {
                return maxDx;
            } else {
                return dx;
            }
        }
    },
    /**
     * 设置行高
     * @param table
     * @param grid
     * @param row
     * @param dy
     */
    setRowHeight: function (table, grid, row, dy) {
        var x, g, cell, maxDy = dy, tmpDy;
        for (x = 0; x < grid[row].length; x++) {
            g = grid[row][x];
            tmpDy = ENV.options.table.rowHeightMin - g.cell.offsetHeight;
            if (g.cell.rowSpan == 1) {
                maxDy = tmpDy;
                cell = g.cell;
                break;
            }
            if (maxDy < tmpDy) {
                maxDy = tmpDy;
                cell = g.cell;
            }
        }

        if (cell) {
            if (dy < maxDy) {
                cell.parentNode.style.height = ENV.options.table.rowHeightMin + 'px';
            } else {
                cell.parentNode.style.height = g.cell.offsetHeight + dy + 'px';
            }
        }
    },
    /**
     * 拆分单元格
     * @param table
     * @param grid
     * @param range
     * @returns {*}
     */
    splitCell: function (table, grid, range) {
        var x, y, g, key, dx, dy;
        var splitMap = tableUtils.canSplit(grid, range);

        if (!splitMap) {
            return null;
        }
        var item, nextCell, newCell;
        for (key in splitMap) {
            if (splitMap.hasOwnProperty(key)) {
                g = splitMap[key];
                dy = g.cell.rowSpan;
                dx = g.cell.colSpan;
                for (y = g.y_src; y < g.y_src + dy; y++) {
                    for (x = g.x_src; x < g.x_src + dx; x++) {
                        item = grid[y][x];
                        if (item.fake) {
                            nextCell = tableUtils.getNextCellDataInRow(grid[y], x);
                            nextCell = nextCell ? nextCell.cell : null;
                            newCell = tableUtils.cloneCell(item.cell, true);
                            table.rows[y].insertBefore(newCell, nextCell);
                            item.fake = false;
                            item.cell = newCell;
                            item.y_src = y;
                            item.x_src = x;
                        } else {
                            item.cell.rowSpan = 1;
                            item.cell.colSpan = 1;
                        }

                        //TODO 需要多做测试检测这样是否可行
                        item.cell.style.width = '';
                    }
                }
            }
        }
        return range;
    }
};

module.exports = tableUtils;
},{"../rangeUtils/rangeExtend":49,"./../common/const":18,"./../common/env":20,"./../common/utils":24,"./../domUtils/domExtend":29}],55:[function(require,module,exports){
/*
 表格选择区域 控制
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    tableUtils = require('./tableUtils'),
    domUtils = require('../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend');

var updateRenderTimer, updateRenderTimes,
    domModifiedTimer;

var zone = {
    active: false,
    table: null,
    start: null,
    end: null,
    range: null,
    grid: null
};
function initZone(table) {
    zone.table = table;
    zone.grid = tableUtils.getTableGrid(zone.table);
}

function clearSelectedCell() {
    if (!zone.table) {
        return;
    }
    var cells = zone.table.getElementsByClassName(CONST.CLASS.SELECTED_CELL);
    var i;
    for (i = cells.length - 1; i >= 0; i--) {
        domUtils.removeClass(cells[i], [CONST.CLASS.SELECTED_CELL, CONST.CLASS.SELECTED_CELL_MULTI]);
    }
}

function getCellsDataByRange() {
    if (!zone.grid || !zone.range) {
        return null;
    }
    var cells = [];
    tableUtils.eachRange(zone.grid, zone.range, function (cellData) {
        if (!cellData.fake) {
            cells.push(cellData);
        }
    });
    return cells;
}
function getDomById(parent, id, tagName) {
    var dom = parent.querySelector('#' + id);
    if (!dom) {
        dom = ENV.doc.createElement(tagName);
        dom.id = id;
        parent.appendChild(dom);
    }
    return dom;
}
function hasMergeCell() {
    if (!zone.grid || !zone.range) {
        return false;
    }
    var hasMerge = false;
    tableUtils.eachRange(zone.grid, zone.range, function (cellData) {
        hasMerge = cellData.fake;
        return !hasMerge;
    });
    return false;
}
function isSingleCell() {
    if (!zone.grid || !zone.range) {
        return false;
    }
    var cellA = zone.grid[zone.range.minY][zone.range.minX],
        cellB = zone.grid[zone.range.maxY][zone.range.maxX],
        start = zone.start;
    return (cellA.cell == cellB.cell && cellB.cell == start.cell);
}
function isStartFocus() {
    var range = rangeUtils.getRange();
    if (!range) {
        //手机端 失去焦点时，不能重新设置焦点，否则会导致无法关闭键盘
        return ENV.client.type.isPhone || ENV.client.type.isPad || !isSingleCell();
    }
    var start, end, endOffset;
    if (zone.grid && zone.start) {
        start = domUtils.getParentByTagName(range.startContainer, ['th', 'td'], true, null);
        end = range.collapsed ? start : domUtils.getParentByTagName(range.endContainer, ['th', 'td'], true, null);
    }

    //当前没有选中单元格  或  当前已选中该单元格时 true
    if (!zone.start || (zone.start.cell == start && start == end)) {
        return true;
    }
    if (!range.collapsed && zone.start.cell == start && start != end &&
        range.endOffset === 0 && end == tableUtils.getNextCellInTable(start)) {
        //如果单元格不是该行最后一个，全选时，endContainer 为下一个 td，且 endOffset 为 0
        //如果单元格是该行最后一个，全选时，endContainer 为下一行的第一个 td
        //这时候必须修正 range， 否则由于 amendUtils.splitAmendDomByRange 的修正，会导致输入的 第1个字符进入到下一个 td 内
        end = start.lastChild;
        endOffset = domUtils.getEndOffset(end);
        //如果不延迟，会导致 选中的区域异常
        setTimeout(function () {
            rangeUtils.setRange(range.startContainer, range.startOffset, end, endOffset);
        }, 200);

        return true;
    }
    return false;
}
function selectCellsData(cellsData) {
    if (!cellsData) {
        return;
    }
    var i, j;
    for (i = 0, j = cellsData.length; i < j; i++) {
        domUtils.addClass(cellsData[i].cell, CONST.CLASS.SELECTED_CELL +
            (j > 1 ? ' ' + CONST.CLASS.SELECTED_CELL_MULTI : ''));
    }
}

function colLineRender(x) {
    if (!zone.table) {
        return;
    }
    var rangeBorder = getRangeBorder();
    var minX = rangeBorder.colLine.minLeft;
    if (x < minX) {
        x = minX;
    }
    domUtils.css(rangeBorder.colLine, {
        top: zone.table.offsetTop + 'px',
        left: x + 'px',
        height: zone.table.offsetHeight + 'px',
        display: 'block'
    });

    rangeBorder.container.style.display = 'block';
}
function rowLineRender(y) {
    if (!zone.table) {
        return;
    }

    var rangeBorder = getRangeBorder();
    var minY = rangeBorder.rowLine.minTop;
    if (y < minY) {
        y = minY;
    }
    domUtils.css(rangeBorder.rowLine, {
        left: zone.table.offsetLeft + 'px',
        top: y + 'px',
        width: zone.table.offsetWidth + 'px',
        display: 'block'
    });

    rangeBorder.container.style.display = 'block';
}

function initTableContainer(rangeBorder) {
    tableUtils.initTableContainer(zone.table);
    var tableBody = zone.table.parentNode;
    tableBody.appendChild(rangeBorder.container);
}

function rangeRender() {
    clearSelectedCell();
    selectCellsData(getCellsDataByRange());

    var rangeBorder = getRangeBorder();
    if (!zone.start || !zone.range) {
        rangeBorder.container.style.display = 'none';
        rangeBorder.start.dom.style.display = 'none';
        rangeBorder.range.dom.style.display = 'none';
        return;
    }
    // console.log(rangeBorder);
    initTableContainer(rangeBorder);

    var topSrc = ENV.doc.body.clientTop;
    var leftSrc = ENV.doc.body.clientLeft;
    var sLeft, sTop, sWidth, sHeight;
    var rLeft, rTop, rWidth, rHeight;

    var rangeCellStart = zone.start ? zone.start.cell : null;
    var rangeCell_A = zone.grid[zone.range.minY][zone.range.minX];
    var rangeCell_B = zone.grid[zone.range.maxY][zone.range.maxX];
    if (!rangeCell_A || !rangeCell_B) {
        return;
    }
    rangeCell_A = rangeCell_A.cell;
    rangeCell_B = rangeCell_B.cell;

    if (rangeCellStart) {
        sTop = topSrc + rangeCellStart.offsetTop;
        sLeft = leftSrc + rangeCellStart.offsetLeft;
        sWidth = rangeCellStart.offsetWidth;
        sHeight = rangeCellStart.offsetHeight;
    }

    rTop = topSrc + rangeCell_A.offsetTop;
    rLeft = leftSrc + rangeCell_A.offsetLeft;
    if (rangeCell_A == rangeCell_B) {
        rWidth = rangeCell_A.offsetWidth;
        rHeight = rangeCell_A.offsetHeight;
    } else {
        rWidth = rangeCell_B.offsetLeft + rangeCell_B.offsetWidth - rLeft;
        rHeight = rangeCell_B.offsetTop + rangeCell_B.offsetHeight - rTop;
    }

    domUtils.css(rangeBorder.start.dom, {
        top: sTop + 'px',
        left: sLeft + 'px'
    });
    domUtils.css(rangeBorder.start.top, {
        width: sWidth + 'px'
    });
    domUtils.css(rangeBorder.start.left, {
        height: sHeight + 'px'
    });
    domUtils.css(rangeBorder.start.bottom, {
        top: (sHeight - 1) + 'px',
        width: sWidth + 'px'
    });
    domUtils.css(rangeBorder.start.right, {
        left: (sWidth - 1) + 'px',
        height: sHeight + 'px'
    });
    domUtils.css(rangeBorder.start.dot, {
        top: (sHeight - 1 - 4) + 'px',
        left: (sWidth - 1 - 4) + 'px'
    });

    domUtils.css(rangeBorder.range.dom, {
        top: rTop + 'px',
        left: rLeft + 'px'
    });
    domUtils.css(rangeBorder.range.top, {
        width: rWidth + 'px'
    });
    domUtils.css(rangeBorder.range.left, {
        height: rHeight + 'px'
    });
    domUtils.css(rangeBorder.range.bottom, {
        top: (rHeight) + 'px',
        width: rWidth + 'px'
    });
    domUtils.css(rangeBorder.range.right, {
        left: (rWidth) + 'px',
        height: rHeight + 'px'
    });
    domUtils.css(rangeBorder.range.dot, {
        top: (rHeight - 4) + 'px',
        left: (rWidth - 4) + 'px'
    });

    rangeBorder.start.dom.style.display = 'block';
    if (isSingleCell()) {
        rangeBorder.start.dot.style.display = 'block';
        rangeBorder.range.dom.style.display = 'none';
    } else {
        rangeBorder.start.dot.style.display = 'none';
        rangeBorder.range.dom.style.display = 'block';
    }
    rangeBorder.container.style.display = 'block';

    //TODO 目前功能未制作，暂时隐藏，以后实现了再显示
    rangeBorder.start.dot.style.display = 'none';
    rangeBorder.range.dot.style.display = 'none';

    setStartRange();
}

function getRangeBorder() {
    var rangeBorder = {
        container: null,
        rowLine: null,
        colLine: null,
        start: {
            dom: null,
            top: null,
            right: null,
            bottom: null,
            left: null,
            dot: null
        },
        range: {
            dom: null,
            top: null,
            right: null,
            bottom: null,
            left: null,
            dot: null
        }
    };
    rangeBorder.container = getDomById(ENV.doc.body, CONST.ID.TABLE_RANGE_BORDER, CONST.TAG.TMP_TAG);
    domUtils.attr(rangeBorder.container, {
        contenteditable: 'false'
    });
    rangeBorder.colLine = getDomById(rangeBorder.container, CONST.ID.TABLE_COL_LINE, 'div');
    rangeBorder.rowLine = getDomById(rangeBorder.container, CONST.ID.TABLE_ROW_LINE, 'div');

    rangeBorder.start.dom = getDomById(rangeBorder.container, CONST.ID.TABLE_RANGE_BORDER + '_start', 'div');
    rangeBorder.start.top = getDomById(rangeBorder.start.dom, CONST.ID.TABLE_RANGE_BORDER + '_start_top', 'div');
    rangeBorder.start.right = getDomById(rangeBorder.start.dom, CONST.ID.TABLE_RANGE_BORDER + '_start_right', 'div');
    rangeBorder.start.bottom = getDomById(rangeBorder.start.dom, CONST.ID.TABLE_RANGE_BORDER + '_start_bottom', 'div');
    rangeBorder.start.left = getDomById(rangeBorder.start.dom, CONST.ID.TABLE_RANGE_BORDER + '_start_left', 'div');
    rangeBorder.start.dot = getDomById(rangeBorder.start.dom, CONST.ID.TABLE_RANGE_BORDER + '_start_dot', 'div');

    rangeBorder.range.dom = getDomById(rangeBorder.container, CONST.ID.TABLE_RANGE_BORDER + '_range', 'div');
    rangeBorder.range.top = getDomById(rangeBorder.range.dom, CONST.ID.TABLE_RANGE_BORDER + '_range_top', 'div');
    rangeBorder.range.right = getDomById(rangeBorder.range.dom, CONST.ID.TABLE_RANGE_BORDER + '_range_right', 'div');
    rangeBorder.range.bottom = getDomById(rangeBorder.range.dom, CONST.ID.TABLE_RANGE_BORDER + '_range_bottom', 'div');
    rangeBorder.range.left = getDomById(rangeBorder.range.dom, CONST.ID.TABLE_RANGE_BORDER + '_range_left', 'div');
    rangeBorder.range.dot = getDomById(rangeBorder.range.dom, CONST.ID.TABLE_RANGE_BORDER + '_range_dot', 'div');
    return rangeBorder;

}
function setStartRange() {
    var sel;
    //选中多个单元格时，取消光标
    if (zone.grid && zone.range && !isSingleCell()) {
        sel = ENV.doc.getSelection();
        sel.empty();
        return;
    }
    if (!isStartFocus()) {
        rangeUtils.setRange(zone.start.cell, zone.start.cell.childNodes.length);
    }
}

var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_SELECTION_CHANGE, _event.handler.onSelectionChange);
        ENV.event.add(CONST.EVENT.UPDATE_RENDER, _event.handler.updateRender);
        if (zone.table) {
            //单元格内插入图片时， 因为加载图片时长为知，触发 modified 的时候， 图片还未加载完毕，
            //但加载完毕后，肯定会触发 body 的 resize事件
            zone.table.addEventListener('DOMSubtreeModified', _event.handler.onDomModified);
            ENV.doc.body.addEventListener('resize', _event.handler.onDomModified);
        }
    },
    unbind: function () {
        var zone = tableZone.getZone();
        ENV.event.remove(CONST.EVENT.ON_SELECTION_CHANGE, _event.handler.onSelectionChange);
        ENV.event.remove(CONST.EVENT.UPDATE_RENDER, _event.handler.updateRender);
        if (zone.table) {
            zone.table.removeEventListener('DOMSubtreeModified', _event.handler.onDomModified);
            ENV.doc.body.removeEventListener('resize', _event.handler.onDomModified);
        }
    },
    bindStopSelectStart: function () {
        _event.unbindStopSelectStart();
        ENV.event.add(CONST.EVENT.ON_SELECT_START, _event.handler.onStopSelectStart);
    },
    unbindStopSelectStart: function () {
        ENV.event.remove(CONST.EVENT.ON_SELECT_START, _event.handler.onStopSelectStart);
    },
    bindDragLine: function () {
        _event.unbindDragLine();
        _event.bindStopSelectStart();
        ENV.event.add(CONST.EVENT.ON_MOUSE_MOVE, _event.handler.onDragLineMove);
        ENV.event.add(CONST.EVENT.ON_MOUSE_UP, _event.handler.onDragLineEnd);
    },
    unbindDragLine: function () {
        _event.unbindStopSelectStart();
        ENV.event.remove(CONST.EVENT.ON_MOUSE_MOVE, _event.handler.onDragLineMove);
        ENV.event.remove(CONST.EVENT.ON_MOUSE_UP, _event.handler.onDragLineEnd);
    },
    handler: {
        onDragLineMove: function (e) {
            var rangeBorder = getRangeBorder();
            var pos = tableUtils.getMousePosition(e, zone.table);
            if (rangeBorder.colLine.style.display == 'block') {
                colLineRender(pos.x - rangeBorder.colLine.startMouse + rangeBorder.colLine.startLine);
            } else {
                rowLineRender(pos.y - rangeBorder.rowLine.startMouse + rangeBorder.rowLine.startLine);
            }
        },
        onDragLineEnd: function (e) {
            _event.unbindDragLine();
            var rangeBorder = getRangeBorder();
            var pos = tableUtils.getMousePosition(e, zone.table);
            var cellData;

            var isDragCol = rangeBorder.colLine.style.display == 'block';
            var isDragRow = rangeBorder.rowLine.style.display == 'block';

            rangeBorder.colLine.style.display = 'none';
            rangeBorder.rowLine.style.display = 'none';
            historyUtils.saveSnap(false);
            if (isDragCol && rangeBorder.colLine.startMouse !== pos.x) {
                cellData = rangeBorder.colLine.cellData;
                if (cellData) {
                    tableUtils.initTable(zone.table);
                    tableUtils.setColWidth(zone.table, zone.grid,
                        cellData.x, pos.x - rangeBorder.colLine.startMouse);
                }
            } else if (isDragRow && rangeBorder.rowLine.startMouse !== pos.y) {
                cellData = rangeBorder.rowLine.cellData;
                if (cellData) {
                    tableUtils.initTable(zone.table);
                    tableUtils.setRowHeight(zone.table, zone.grid,
                        cellData.y, pos.y - rangeBorder.rowLine.startMouse);
                }
            }

            rangeBorder.colLine.cellData = null;
            rangeBorder.colLine.minLeft = null;
            rangeBorder.colLine.startLine = null;
            rangeBorder.colLine.startMouse = null;
            rangeBorder.rowLine.cellData = null;
            rangeBorder.rowLine.minTop = null;
            rangeBorder.rowLine.startLine = null;
            rangeBorder.rowLine.startMouse = null;

            ENV.event.call(CONST.EVENT.UPDATE_RENDER);
        },
        onSelectionChange: function (e) {
            //当选中单元格时，不允许 选中 start 单元格以外的任何内容
            var sel = ENV.doc.getSelection();
            if (!isStartFocus()) {
                sel.empty();
                // rangeUtils.setRange(zone.start.cell, zone.start.cell.childNodes.length);
                utils.stopEvent(e);
            }
        },
        onDomModified: function (e) {
            var needAutoRetry = (e && e.type == 'DOMSubtreeModified' && e.target.nodeType === 1 && e.target.querySelector('img'));
            if (domModifiedTimer) {
                clearTimeout(domModifiedTimer);
            }
            domModifiedTimer = setTimeout(function () {
                ENV.event.call(CONST.EVENT.UPDATE_RENDER, e, needAutoRetry);
            }, 100);
        },
        onStopSelectStart: function (e) {
            utils.stopEvent(e);
            return false;
        },
        updateRender: function (e, needAutoRetry) {
            updateRenderTimes = 0;
            autoUpdate(needAutoRetry);

            function autoUpdate(needAutoRetry) {
                //单元格内容变化时，必须重绘，保证 高亮边框的高度与表格一致
                rangeRender();
                //如果 变化的内容里面有 img，则需要延迟监听，等 img 渲染完毕
                if (needAutoRetry && updateRenderTimes < 60) {
                    if (updateRenderTimer) {
                        clearTimeout(updateRenderTimer);
                    }
                    updateRenderTimer = setTimeout(function () {
                        updateRenderTimes++;
                        autoUpdate(needAutoRetry);
                    }, 500);
                }
            }
        }
    }
};

var tableZone = {
    clear: function () {
        zone.active = false;
        zone.start = null;
        zone.end = null;
        zone.range = null;
        zone.grid = null;

        rangeRender();

        var rangeBorder = getRangeBorder();
        rangeBorder.colLine.style.display = 'none';
        rangeBorder.rowLine.style.display = 'none';

        //table 必须最后清空，因为还要清除 table 里面 cell 的选择状态
        zone.table = null;
        _event.unbind();
        return tableZone;
    },
    /**
     * 为 复制/剪切 操作，准备 fragment
     */
    getFragmentForCopy: function () {
        var fragment = null;
        //无选中单元格时，不进行任何操作
        if (!zone.range) {
            return fragment;
        }

        var x, y, g,
            table = ENV.doc.createElement('table'),
            tbody = ENV.doc.createElement('tbody'),
            tr, td;

        table.appendChild(tbody);
        for (y = zone.range.minY; y <= zone.range.maxY; y++) {
            tr = ENV.doc.createElement('tr');
            for (x = zone.range.minX; x <= zone.range.maxX; x++) {
                g = zone.grid[y][x];
                if (!g.fake) {
                    td = tableUtils.cloneCell(g.cell, false);
                    if (tr.children.length > 0) {
                        //保证 复制的纯文本 有 列间隔
                        tr.appendChild(ENV.doc.createTextNode('\u0009'));
                    }
                    tr.appendChild(td);
                }
            }
            //保证 复制的纯文本 有 行间隔
            tr.appendChild(ENV.doc.createTextNode('\n'));
            tbody.appendChild(tr);
        }

        fragment = ENV.doc.createElement('div');
        fragment.appendChild(table);
        return fragment;
    },
    getRangeBorder: getRangeBorder,
    getSelectedCells: function () {
        return tableUtils.getCellsByRange(zone.grid, zone.range);
    },
    getZone: function () {
        return {
            active: zone.active,
            table: zone.table,
            start: zone.start,
            end: zone.end,
            range: zone.range,
            grid: zone.grid
        };
    },
    hasMergeCell: hasMergeCell,
    isRangeActiving: function () {
        return zone.start && zone.active;
    },
    isSingleCell: isSingleCell,
    isZoneBorder: function (e) {
        var obj = e.target, x = e.offsetX, y = e.offsetY,
            eventClient = utils.getEventClientPos(e);
        var isScroll,
            isBodyBorder = false,
            isBorder = false,
            isRight = false,
            isBottom = false,
            isContainer = false;

        var isDot = !!domUtils.getParentByFilter(obj, function (dom) {
            return dom && dom.nodeType == 1 &&
                (dom.id == CONST.ID.TABLE_RANGE_BORDER + '_start_dot' || dom.id == CONST.ID.TABLE_RANGE_BORDER + '_range_dot');
        }, true);

        if (!isDot) {
            isRight = !!domUtils.getParentByFilter(obj, function (dom) {
                if (dom && dom.nodeType == 1 &&
                    (dom.id == CONST.ID.TABLE_RANGE_BORDER + '_start_right' || dom.id == CONST.ID.TABLE_RANGE_BORDER + '_range_right')) {
                    return true;
                }

                var minX, maxX;
                if (dom && dom.nodeType == 1 && domUtils.isTag(dom, ['td', 'th'])) {
                    minX = dom.offsetWidth - 4;
                    maxX = dom.offsetWidth + 4;
                    return (x >= minX && x <= maxX);
                }
                return false;
            }, true);
        }
        if (!isDot && !isRight) {
            isBottom = !!domUtils.getParentByFilter(obj, function (dom) {
                if (dom && dom.nodeType == 1 &&
                    (dom.id == CONST.ID.TABLE_RANGE_BORDER + '_start_bottom' || dom.id == CONST.ID.TABLE_RANGE_BORDER + '_range_bottom')) {
                    return true;
                }

                var minY, maxY;
                if (dom && dom.nodeType == 1 && domUtils.isTag(dom, ['td', 'th'])) {
                    minY = dom.offsetHeight - 4;
                    maxY = dom.offsetHeight + 4;
                    return (y >= minY && y <= maxY);
                }
                return false;
            }, true);
        }
        if (!isBottom && !isDot && !isRight) {
            isBorder = !!domUtils.getParentByFilter(obj, function (dom) {
                return dom && dom.nodeType == 1 && dom.id == CONST.ID.TABLE_RANGE_BORDER;
            }, true);
        }

        var bodyStyle, bodyLeft, bodyRight;
        if (!isBottom && !isDot && !isRight && !isBorder) {
            isContainer = !!tableUtils.getContainerExcludeTable(obj);
            if (!isContainer && obj == ENV.doc.body) {
                bodyStyle = ENV.win.getComputedStyle(obj);
                bodyLeft = parseInt(bodyStyle.paddingLeft);
                bodyRight = parseInt(bodyStyle.paddingRight);
                isBodyBorder = eventClient.x <= bodyLeft || eventClient.x >= (ENV.doc.body.offsetWidth - bodyRight);
            }
        }

        //span 等 行级元素 clientWidth / clientHeight 为 0
        isScroll = ((e.target.clientWidth > 0 && e.target.clientWidth < e.offsetX) ||
            (e.target.clientHeight > 0 && e.target.clientHeight < e.offsetY)) &&
            (e.target.offsetWidth >= e.offsetX || e.target.offsetHeight >= e.offsetY);

        return {
            isBodyBorder: isBodyBorder,
            isBorder: isBorder,
            isBottom: isBottom,
            isContainer: isContainer,
            isDot: isDot,
            isRight: isRight,
            isScroll: isScroll
        };
    },
    modify: function (endCell) {
        if (!zone.active || !endCell) {
            return tableZone;
        }
        // console.log('modify');
        var table = domUtils.getParentByTagName(endCell, ['table'], true, null);
        if (!table || table !== zone.table) {
            return tableZone;
        }
        var endCellData = tableUtils.getCellData(zone.grid, endCell);
        zone.range = tableUtils.getRangeByCellsData(zone.grid, zone.start, endCellData);
        zone.end = endCellData;

        rangeRender();

        var tableBody = domUtils.getParentByFilter(zone.table, function (dom) {
            return domUtils.hasClass(dom, CONST.CLASS.TABLE_BODY);
        }, false);
        domUtils.addClass(tableBody, CONST.CLASS.TABLE_MOVING);

        return tableZone;
    },
    remove: function () {
        tableZone.clear();
        var rangeBorder = getRangeBorder(), parent;
        if (rangeBorder) {
            parent = rangeBorder.container.parentNode;
            if (parent) {
                parent.removeChild(rangeBorder.container);
            }
        }
    },
    setEnd: function (endCell, isForced) {
        // console.log('setEnd');
        if (isForced) {
            zone.active = true;
        }
        tableZone.modify(endCell);
        zone.active = false;

        setStartRange();
        ENV.event.call(CONST.EVENT.ON_SELECTION_CHANGE, null);

        var tableBody = domUtils.getParentByFilter(zone.table, function (dom) {
            return domUtils.hasClass(dom, CONST.CLASS.TABLE_BODY);
        }, false);
        domUtils.removeClass(tableBody, CONST.CLASS.TABLE_MOVING);

        return tableZone;
        // console.log(zone);
    },
    setStart: function (startCell, curX, curY) {
        // console.log('setStart');
        if (!startCell) {
            tableZone.clear();
            return tableZone;
        }
        var table = domUtils.getParentByTagName(startCell, ['table'], true, null);
        if (!table) {
            //防止异常的 标签
            tableZone.clear();
            return tableZone;
        }
        if (table !== zone.table) {
            tableZone.clear();
            initZone(table);
        }
        zone.active = true;
        zone.end = null;
        zone.start = tableUtils.getCellData(zone.grid, startCell);
        if (typeof curX !== 'undefined' && typeof curY !== 'undefined') {
            try {
                var tmp = zone.grid[curY][curX];
                if (tmp && tmp.cell == zone.start.cell) {
                    zone.start = tmp;
                }
            } catch (e) {
            }
        }
        zone.range = tableUtils.getRangeByCellsData(zone.grid, zone.start, zone.start);

        rangeRender();
        if (zone.start.cell.scrollIntoViewIfNeeded) {
            zone.start.cell.scrollIntoViewIfNeeded();
        }

        _event.bind();
        return tableZone;
    },
    setStartRange: setStartRange,
    startDragColLine: function (cell, x) {
        var table, cellData;
        if (cell && cell.nodeType == 1 && cell.id == CONST.ID.TABLE_RANGE_BORDER + '_start_right') {
            cellData = zone.start;
            cell = zone.start.cell;
            table = zone.table;
        } else if (cell && cell.nodeType == 1 && cell.id == CONST.ID.TABLE_RANGE_BORDER + '_range_right') {
            cellData = zone.grid[zone.range.maxY][zone.range.maxX];
            cell = cellData.cell;
            table = zone.table;
        } else {
            cell = domUtils.getParentByTagName(cell, ['th', 'td'], true, null);
            if (!cell) {
                return;
            }
            table = domUtils.getParentByTagName(cell, ['table'], true, null);
            if (!table) {
                return;
            }

            if (table !== zone.table) {
                clearSelectedCell();
                tableZone.clear();
            }
            if (!zone.grid) {
                initZone(table);
            }
            cellData = tableUtils.getCellData(zone.grid, cell);
        }

        //如果 cell 是合并的单元格，需要找到 cell 所占的最后一列
        var col = cellData.x, nextCellData;
        while (col + 1 < zone.grid[cellData.y].length) {
            col++;
            nextCellData = zone.grid[cellData.y][col];
            if (nextCellData.cell != cell) {
                break;
            }
            cellData = nextCellData;
        }

        var startLeft = cell.offsetLeft + cell.offsetWidth;
        var rangeBorder = getRangeBorder();
        initTableContainer(rangeBorder);
        rangeBorder.colLine.minLeft = table.offsetLeft;
        rangeBorder.colLine.startLine = startLeft;
        rangeBorder.colLine.startMouse = x;
        rangeBorder.colLine.cellData = cellData;
        colLineRender(startLeft);

        var sel = ENV.doc.getSelection();
        sel.empty();
        _event.bindDragLine();

    },
    startDragRowLine: function (cell, y) {
        var table, cellData;
        if (cell && cell.nodeType == 1 && cell.id == CONST.ID.TABLE_RANGE_BORDER + '_start_bottom') {
            cellData = zone.start;
            cell = zone.start.cell;
            table = zone.table;
        } else if (cell && cell.nodeType == 1 && cell.id == CONST.ID.TABLE_RANGE_BORDER + '_range_bottom') {
            cellData = zone.grid[zone.range.maxY][zone.range.maxX];
            cell = cellData.cell;
            table = zone.table;
        } else {
            cell = domUtils.getParentByTagName(cell, ['th', 'td'], true, null);
            if (!cell) {
                return;
            }
            table = domUtils.getParentByTagName(cell, ['table'], true, null);
            if (!table) {
                return;
            }

            if (table !== zone.table) {
                clearSelectedCell();
                tableZone.clear();
            }
            if (!zone.grid) {
                initZone(table);
            }
            cellData = tableUtils.getCellData(zone.grid, cell);
        }

        //如果 cell 是合并的单元格，需要找到 cell 所占的最后一行
        var row = cellData.y, nextCellData;
        while (row + 1 < zone.grid.length) {
            row++;
            nextCellData = zone.grid[row][cellData.x];
            if (nextCellData.cell != cell) {
                break;
            }
            cellData = nextCellData;
        }

        var startTop = cell.offsetTop + cell.offsetHeight;
        var rangeBorder = getRangeBorder();
        initTableContainer(rangeBorder);
        rangeBorder.rowLine.minTop = table.offsetTop;
        rangeBorder.rowLine.startLine = startTop;
        rangeBorder.rowLine.startMouse = y;
        rangeBorder.rowLine.cellData = cellData;
        rowLineRender(startTop);

        var sel = ENV.doc.getSelection();
        sel.empty();
        _event.bindDragLine();

    },
    switchCell: function (target, direct) {
        if (!direct || !zone.start) {
            return null;
        }
        //目前不考虑 x、y 为任意值的情况， 只考虑移动一个单元格
        direct.x = !direct.x ? 0 : (direct.x > 0 ? 1 : -1);
        direct.y = !direct.y ? 0 : (direct.y > 0 ? 1 : -1);
        var x = target.x + direct.x;
        var y = target.y + direct.y;

        changeRowCheck();

        var cellData = target;
        while (y >= 0 && y < zone.grid.length &&
        x >= 0 && x < zone.grid[y].length &&
        cellData.cell == target.cell) {

            cellData = zone.grid[y][x];
            x += direct.x;
            y += direct.y;

            changeRowCheck();
        }

        return cellData;

        function changeRowCheck() {
            if (!!direct.canChangeRow && y >= 0 && y < zone.grid.length) {
                //允许折行
                if (x < 0) {
                    x = zone.grid[y].length - 1;
                    y -= 1;
                } else if (x >= zone.grid[y].length) {
                    x = 0;
                    y += 1;
                }
            }
        }
    },
    updateGrid: function () {
        var rangeA, rangeB;
        if (zone.table) {
            if (zone.grid) {
                rangeA = zone.grid[zone.range.minY][zone.range.minX];
                rangeB = zone.grid[zone.range.maxY][zone.range.maxX];
            }
            initZone(zone.table);
            rangeA = tableUtils.getCellData(zone.grid, rangeA.cell);
            rangeB = tableUtils.getCellData(zone.grid, rangeB.cell);
            zone.range = tableUtils.getRangeByCellsData(zone.grid, rangeA, rangeB);
            zone.start = tableUtils.getCellData(zone.grid, zone.start.cell);
            if (zone.end) {
                zone.end = tableUtils.getCellData(zone.grid, zone.end.cell);
            }
        }
        ENV.event.call(CONST.EVENT.UPDATE_RENDER);

        return tableZone;
        // console.log(zone);
    }
};

module.exports = tableZone;

},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"./tableUtils":54}],56:[function(require,module,exports){
/**
 * todolist 操作核心包 core
 */
var ENV = require('../common/env'),
    CONST = require('../common/const'),
    utils = require('../common/utils'),
    historyUtils = require('../common/historyUtils'),
    domUtils = require('../domUtils/domExtend'),
    rangeUtils = require('../rangeUtils/rangeExtend'),
    todoRouteForClient = require('./todoRouteForClient'),
    todoUtils = require('./todoUtils'),
    todoStyle = require('./todoStyle');

var todoRoute = null;
var curTouchTarget = null;

var patchForReader = {
    curCheckbox: null,
    docLockChecked: false,
    modifiedIdList: {},
    htmlToSave: '',
    init: function () {
        if (ENV.readonly) {
            patchForReader.modifiedIdList = {};
            var i, checkItem;
            var checkList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TODO_CHECKBOX);
            // 1. 旧版的 todoList checkbox 无 id
            // 2. 新版的 todoList 有可能出现 id 重复
            // 所以阅读模式下判定点击的 checkbox 只能依靠序号进行
            for (i = 0; i < checkList.length; i++) {
                checkItem = checkList[i];
                checkItem.wizTodoIndex = i;
            }
        }
    },
    addModifiedId: function (checkResult) {
        if (!ENV.readonly) {
            return;
        }
        var checkbox = checkResult.checkbox;
        var curChecked = checkResult.checked;
        var lastChecked = patchForReader.modifiedIdList[checkbox.wizTodoIndex];
        if (typeof lastChecked === 'boolean' && lastChecked !== curChecked) {
            delete patchForReader.modifiedIdList[checkbox.wizTodoIndex];
        } else {
            patchForReader.modifiedIdList[checkbox.wizTodoIndex] = curChecked;
        }
    },
    getModifiedIdList: function () {
        var k, idList = [];
        for (k in patchForReader.modifiedIdList) {
            if (patchForReader.modifiedIdList.hasOwnProperty(k)) {
                idList.push({
                    id: k,
                    checked: patchForReader.modifiedIdList[k]
                });
            }
        }
        return idList;
    },
    modifyDoc: function () {
        var html = '';

        var idList = patchForReader.getModifiedIdList();
        if (idList.length === 0) {
            return html;
        }

        html = todoRoute.getOriginalDoc();

        var iframe = ENV.doc.getElementById(CONST.ID.IFRAME_FOR_SAVE);
        if (!iframe) {
            iframe = ENV.doc.createElement('iframe');
            iframe.id = CONST.ID.IFRAME_FOR_SAVE;
            ENV.doc.body.appendChild(iframe);
        }
        iframe.style.display = 'none';

        var _document = ENV.doc,
            _win = ENV.win,
            iframeDocument = iframe.contentDocument,
            isPersonal = todoRoute.isPersonalDocument(),
            i, id, checked, checkList, checkbox, main;

        iframeDocument.open("text/html", "replace");
        iframeDocument.write(html);
        iframeDocument.close();

        ENV.doc = iframeDocument;
        ENV.win = iframe.contentWindow;

        //在 iframe 内处理 html
        todoUtils.oldPatch.fixOldTodo();
        todoUtils.fixNewTodo();

        checkList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TODO_CHECKBOX);
        for (i = idList.length - 1; i >= 0; i--) {
            id = idList[i].id;
            checked = idList[i].checked;
            checkbox = checkList[id];
            if (checkbox) {
                main = todoUtils.getMainFromChild(checkbox);
                todoUtils.check(main, checked);
                if (!isPersonal) {
                    todoUtils.addUserInfo(main, checked, checkbox.id, todoRoute);
                }
            }
        }
        todoUtils.checkTodoStyle(true);

        //获取处理后的 html
        html = domUtils.getContentHtml();
        ENV.doc = _document;
        ENV.win = _win;

        ENV.doc.body.removeChild(iframe);
        return html;
    }
};

function onClickTodoCheck (e) {
    if (!todoRoute.hasPermission()) {
        return null;
    }
    var checkbox = e.target;
    if (!domUtils.hasClass(checkbox, CONST.CLASS.TODO_CHECKBOX)) {
        return null;
    }
    var layer = todoUtils.getContainerFromChild(checkbox);
    var parentlayer = todoUtils.getContainerFromChild(layer.parentNode);
    if (parentlayer) {
        todoUtils.fixNewTodo(parentlayer);
    }

    if (ENV.readonly && !!patchForReader.curCheckbox) {
        return null;
    }
    if (!ENV.readonly || patchForReader.docLockChecked) {
        checkTodo(checkbox, e);
    } else {
        patchForReader.curCheckbox = checkbox;
        todoRoute.checkDocLock("onCheckDocLock");
    }
}

function checkTodo (checkbox, e) {
    var result = todoUtils.checkTodo(checkbox, todoRoute);
    if (result) {
        if (ENV.readonly) {
            patchForReader.addModifiedId(result);
        }
        if (e) {
            utils.stopEvent(e);
        }
    }
}

//TODO 所有配色 要考虑到 夜间模式
var _event = {
    bind: function () {
        _event.unbind();
        ENV.event.add(CONST.EVENT.ON_SELECTION_CHANGE, _event.handler.onSelectionChange);
        ENV.event.add(CONST.EVENT.AFTER_RESTORE_HISTORY, _event.handler.afterRestoreHistory);
        ENV.event.add(CONST.EVENT.ON_EXEC_COMMAND, _event.handler.onExecCommand);
        ENV.event.add(CONST.EVENT.ON_PASTE, _event.handler.onPaste);
        ENV.event.add(CONST.EVENT.AFTER_INSERT_DOM, _event.handler.afterInsertDom); // ios 端 paste 时使用的是 inertHtml 方法

        if (ENV.client.type.isIOS || ENV.client.type.isAndroid) {
            ENV.event.add(CONST.EVENT.ON_TOUCH_END, _event.handler.onTouchEnd);
            ENV.event.add(CONST.EVENT.ON_TOUCH_START, _event.handler.onTouchStart);
        } else {
            ENV.event.add(CONST.EVENT.ON_CLICK, _event.handler.onClick);
        }
    },
    unbind: function () {
        ENV.event.remove(CONST.EVENT.ON_CLICK, _event.handler.onClick);
        ENV.event.remove(CONST.EVENT.ON_SELECTION_CHANGE, _event.handler.onSelectionChange);
        ENV.event.remove(CONST.EVENT.ON_TOUCH_END, _event.handler.onTouchEnd);
        ENV.event.remove(CONST.EVENT.ON_TOUCH_START, _event.handler.onTouchStart);
        ENV.event.remove(CONST.EVENT.AFTER_RESTORE_HISTORY, _event.handler.afterRestoreHistory);
    },
    handler: {
        afterRestoreHistory: function () {
            //恢复历史后，需要检查 todoList 的 style 样式
            todoUtils.checkTodoStyle(false);
            todoUtils.fixNewTodo();
            todoStyle.restoreUserAvatarStyle();
        },
        onClick: function (e) {
            onClickTodoCheck(e);
        },
        onCheckDocLock: function (cancel, needCallAgain) {
            patchForReader.docLockChecked = !needCallAgain;
            if (!cancel) {
                checkTodo(patchForReader.curCheckbox);
            }
            patchForReader.curCheckbox = null;
        },
        onExecCommand: function () {
            var range = rangeUtils.getRange(),
                parent, checkList,
                i, j;
            if (!range) {
                return;
            }
            parent = range.collapsed ? range.startContainer : domUtils.getParentRoot([range.startContainer, range.endContainer]);
            parent = domUtils.getBlockParent(parent, true);
            if (!parent) {
                return;
            }
            checkList = parent.querySelectorAll('.' + CONST.CLASS.TODO_CHECKBOX);
            for (i = 0, j = checkList.length; i < j; i++) {
                todoUtils.fixCheckbox(checkList[i], false);
            }
        },
        onKeyDown: function (e) {
            if (!todoRoute.hasPermission()) {
                return true;
            }
            var range = rangeUtils.getRange();
            if (!range) {
                return true;
            }

            var keyCode = e.keyCode || e.which;

            var start, startOffset, end, endOffset,
                isAfterCheck, main, container,
                tmpMain, tmpContainer, tmpEndContainer,
                isLineEnd, isEmpty,
                mainParentTag, mainParent,
                childNodes, i, dom;

            var rangeList;
            if (!range.collapsed) {
                if (keyCode !== 8 && keyCode !== 46 && utils.checkNonTxtKey(e)) {
                    return true;
                }

                //将 选中的 checkbox cancelTodo
                rangeList = rangeUtils.getRangeDomList({
                    noSplit: true
                });
                if (rangeList && rangeList.list.length > 0) {

                    historyUtils.saveSnap(false);

                    end = range.endContainer;
                    endOffset = range.endOffset;
                    if (end.nodeType !== 3) {
                        if (range.endOffset === end.childNodes.length) {
                            end = domUtils.getLastDeepChild(end);
                            endOffset = domUtils.getEndOffset(end);
                        } else {
                            end = end.childNodes[range.endOffset];
                            endOffset = 0;
                        }
                    }
                    tmpEndContainer = todoUtils.getContainerFromChild(end);

                    for (i = rangeList.list.length - 1; i >= 0; i--) {
                        dom = rangeList.list[i];
                        if (todoUtils.isCheckbox(dom)) {
                            tmpContainer = todoUtils.getContainerFromChild(dom);
                            tmpMain = todoUtils.getMainFromChild(dom);
                            if (tmpMain) {
                                container = tmpMain.parentNode;
                                todoUtils.cancelTodo(container, true);
                            }
                            //如果 todoList 是 光标范围最后一行，则需要修正 range
                            if (tmpEndContainer && tmpContainer == tmpEndContainer) {
                                if (!end.parentNode) {
                                    end = container;
                                    endOffset = domUtils.getEndOffset(end);
                                }
                                rangeUtils.setRange(range.startContainer, range.startOffset, end, endOffset);
                            }
                        }
                    }
                    return true;
                }
                range.collapse(true);
            }

            isAfterCheck = todoUtils.isCaretAfterCheckbox();
            main = todoUtils.getMainByCaret();
            container = main ? domUtils.getBlockParent(main, false) : todoUtils.getContainerFromChild(range.startContainer);
            isLineEnd = domUtils.getEndOffset(range.endContainer) === range.endOffset;

            // 如果只有一个空的 container，直接删除 class
            if (container && !main && !todoUtils.getCheckbox(container)) {
                domUtils.removeClass(container, CONST.CLASS.TODO_LAYER);
                container = null;
            }

            //isLineEnd 主要用于处理 在行尾 按下 delete 键
            // 回车键 必须要进入后面进行处理
            if (!container && !isLineEnd && keyCode !== 13) {
                return true;
            }

            /**
             * Backspace
             */
            if (keyCode === 8 && isAfterCheck) {
                historyUtils.saveSnap(false);
                todoUtils.cancelTodo(container);
                utils.stopEvent(e);
                return false;
            }

            /**
             * Delete
             */
            if (keyCode === 46) {
                historyUtils.saveSnap(false);
                start = range.startContainer;
                startOffset = range.startOffset;
                rangeUtils.selectCharIncludeFillChar();
                range = rangeUtils.getRange();
                end = range.endContainer;
                if (range.endOffset === 0) {
                    // 避免 br 后直接是 todoList 时，直接把 br 后面的 todoList 删除
                    end = domUtils.getPreviousNode(end);
                }
                tmpMain = todoUtils.getMainFromChild(end) || todoUtils.getMainInDom(end);
                //恢复 range
                rangeUtils.setRange(start, startOffset);
                if (tmpMain && tmpMain != main) {
                    todoUtils.cancelTodo(tmpMain.parentNode, true);
                    return false;
                }
            }

            /**
             * left
             */
            if (keyCode === 37 && isAfterCheck) {
                dom = domUtils.getPreviousNode(container);
                if (dom) {
                    rangeUtils.setRange(dom, domUtils.getEndOffset(dom));
                    utils.stopEvent(e);
                    return false;
                }
            }
            // /**
            //  * right
            //  */
            // if (keyCode === 39) {
            //     return;
            // }
            if (!container || keyCode !== 13 || e.shiftKey) {
                return true;
            }

            historyUtils.saveSnap(false);

            // console.log('isEmptyContainer: ' + todoUtils.isEmptyContainer(container));
            if (todoUtils.isEmptyContainer(container)) {
                if (domUtils.getParentByTagName(container, ['blockquote', 'ul', 'ol'])) {
                    //如果当前 todoList 为空，且 todoList 处于 blockquote、ul、ol 内时执行减少缩进操作
                    ENV.event.call(CONST.EVENT.EXEC_COMMEND, 'outdent', false);
                } else {
                    //如果当前 todoList 为空，则 取消 todoList
                    container.innerHTML = '<br>';
                    domUtils.removeClass(container, CONST.CLASS.TODO_LAYER);
                    rangeUtils.setRange(container, 1);
                }
                utils.stopEvent(e);
                return false;
            }

            range.deleteContents();
            todoUtils.fixNewTodo(container);
            //判断光标是否在 main 内
            var isInMain = false;
            if (main) {
                if (range.startContainer == main && range.startOffset !== domUtils.getEndOffset(main)) {
                    isInMain = true;
                } else if (todoUtils.getMainFromChild(range.startContainer) &&
                    (range.startOffset !== domUtils.getEndOffset(range.startContainer) ||
                    range.startContainer.nextSibling)) {
                    isInMain = true;
                }
            }
            range.setEndAfter(isInMain ? main : container);
            var tmpRange = range.cloneContents();
            var tmpDiv = ENV.doc.createElement('div');
            tmpRange.appendChild(tmpDiv);
            while (tmpRange.childNodes.length > 1) {
                tmpDiv.appendChild(tmpRange.firstChild);
            }
            isEmpty = domUtils.isEmptyDom(tmpDiv);
            var frag;
            if (!isEmpty) {
                frag = range.extractContents();
                childNodes = [];
                for (i = 0; i < frag.childNodes.length; i++) {
                    childNodes.push(frag.childNodes[i]);
                }
                mainParentTag = (container === ENV.doc.body || !domUtils.isBlock(container)) ? 'div' : container.tagName;
                mainParent = ENV.doc.createElement(mainParentTag);
                domUtils.after(mainParent, container);
                main = todoUtils.setTodo(mainParent, todoRoute);
                todoUtils.insertToMain(childNodes, main);

                // 如果换行操作的当前 块元素是 todoList，则 frag 内会自动生成 main
                // 必须要插入到 document 内才能正常清理 frag 内 main & block 元素
                for (i = 0; i < childNodes.length; i++) {
                    todoUtils.clearBlock(childNodes[i]);
                }
                rangeUtils.setRange(main, 1);
            } else {
                mainParent = todoUtils.cloneTodo(container);
                main = todoUtils.getMainInDom(mainParent);
                domUtils.after(mainParent, container);
                todoUtils.setTodo(mainParent, todoRoute);
                dom = domUtils.getLastDeepChild(mainParent);
                if (dom.nodeType === 3) {
                    dom.nodeValue = CONST.FILL_CHAR + CONST.FILL_CHAR;
                } else {
                    dom.appendChild(ENV.doc.createTextNode(CONST.FILL_CHAR + CONST.FILL_CHAR));
                    dom = dom.childNodes[0];
                }
                rangeUtils.setRange(dom, 1);
            }

            utils.stopEvent(e);

            if (main.getBoundingClientRect().top + main.clientHeight > ENV.doc.documentElement.clientHeight
                || main.getBoundingClientRect().top + main.clientHeight < 0) {
                var mainX = main.getBoundingClientRect().left + ENV.doc.body.scrollLeft;
                var mainY = ENV.doc.body.scrollTop + main.clientHeight;
                window.scrollTo(mainX, mainY);
            }
        },
        afterInsertDom: function () {
            //粘贴后 需要修正 样式以及 dom 结构
            todoUtils.fixNewTodo();
            todoUtils.checkTodoStyle(false);
        },
        onPaste: function () {
            setTimeout(function () {
                _event.handler.afterInsertDom();
            }, 300);
        },
        onSelectionChange: function (e) {
            var range = rangeUtils.getRange();
            if (!range) {
                return;
            }
            var checkbox,
                end, endOffset;
            if (!range.collapsed) {
                end = range.endContainer;
                endOffset = range.endOffset;
            }
            var isBefore = todoUtils.isCaretBeforeCheckbox();
            if (isBefore.enable) {
                checkbox = isBefore.checkbox;
                if (checkbox && checkbox.nextSibling) {
                    end = checkbox.nextSibling;
                    endOffset = 0;
                } else if (checkbox) {
                    end = checkbox.parentNode;
                    endOffset = domUtils.getIndex(checkbox) + 1;
                }

                if (!range.collapsed) {
                    rangeUtils.setRange(range.startContainer, range.startOffset, end, endOffset);
                } else {
                    rangeUtils.setRange(end, endOffset);
                }
            }
        },
        onTouchStart: function (e) {
            curTouchTarget = e.target;
        },
        onTouchEnd: function (e) {
            if (e.target !== curTouchTarget) {
                return;
            }
            curTouchTarget = null;
            onClickTodoCheck(e);
        }
    }
};

var todoCore = {
    init: function () {
    },
    on: function () {
        todoUtils.oldPatch.fixOldTodo();
        todoUtils.fixNewTodo();
        patchForReader.init();
        _event.bind();
        if (!todoRoute) {
            todoRoute = todoRouteForClient.getRoute();
        }
        todoUtils.checkTodoStyle(true);
    },
    off: function () {
        _event.unbind();
    },
    checkTodoStyle: function () {
        todoUtils.checkTodoStyle(false);
    },
    closeDocument: function () {
        var html;

        if (!todoRoute) {
            todoRoute = todoRouteForClient.getRoute();
        }

        /**
         * ENV.options.pc.pluginModified 是 PC 客户端 插件在阅读状态下修改笔记的标识
         * markdown、mathjax 笔记都不允许进行此操作（由插件自行处理）
         * 普通笔记可以直接获取当前阅读页面的 html 进行保存
         */
        if (ENV.client.type.isWin && ENV.options.pc.pluginModified) {
            html = domUtils.getContentHtml();
        } else {
            html = patchForReader.modifyDoc();
        }

        if (!!html && !ENV.client.type.isIOS && !ENV.client.type.isMac) {
            todoRoute.saveDoc(html, '');
        }

        if (todoRoute.beforeCloseDoc) {
            todoRoute.beforeCloseDoc();
        }

        patchForReader.init();
        return html;
    },
    onCheckDocLock: _event.handler.onCheckDocLock,
    onKeyDown: _event.handler.onKeyDown,
    setTodo: function () {
        historyUtils.saveSnap(false);
        todoUtils.setTodo(null, todoRoute);
    },
    setTodoInfo: function (options) {
        if (todoRoute.setTodoInfo) {
            todoRoute.setTodoInfo(options);
        }
    }
};

module.exports = todoCore;

},{"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/utils":24,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"./todoRouteForClient":57,"./todoStyle":58,"./todoUtils":59}],57:[function(require,module,exports){
/**
 * todolist 客户端适配接口
 */
var ENV = require('../common/env'),
    base64 = require('../common/base64');

var winExternal, qtEditor, androidWizNote;

function routeForWindows() {

    winExternal = ENV.win.external;

    this.getUserAlias = getUserAlias;
    this.getUserGuid = getUserGuid;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.setDocumentType = setDocumentType;
    this.hasPermission = hasPermission;
    //for Reader
    this.getOriginalDoc = getOriginalDoc;
    this.saveDoc = saveDoc;
    this.checkDocLock = checkDocLock;

    function getUserAlias() {
        try {
            return winExternal.UserAlias;
        } catch (e) {
            console.error(e);
            return '';
        }
    }

    function getUserGuid() {
        try {
            return winExternal.GetUserGUID();
        } catch (e) {
            console.error(e);
            return '';
        }
    }

    function getUserAvatarFileName(size) {
        try {
            return winExternal.GetUserAvatarFileName(size);
        } catch (e) {
            console.error(e);
            return '';
        }
    }

    function isPersonalDocument() {
        try {
            return winExternal.WizDocument.IsPersonalDocument();
        } catch (e) {
            console.error(e);
            return false;
        }
    }

    function setDocumentType(type) {
        /*
         var oldType = this.wizDoc.Type;
         if (oldType) {
         if (-1 == oldType.indexOf(type)) {
         this.wizDoc.Type = oldType + ';' + type;
         }
         }
         else {
         this.wizDoc.Type = type;
         }*/
        if (winExternal.WizDocument) {
            winExternal.WizDocument.Type = type;
        }
    }

    function hasPermission() {
        try {
            return !ENV.readonly || winExternal.WizDocument.CanEdit;
        } catch (e) {
            console.error(e);
            return false;
        }
    }

    function getOriginalDoc() {
        try {
            return winExternal.WizDocument.GetHtml();
        } catch (e) {
            console.error(e);
            return '';
        }
    }

    function saveDoc(html, resources) {
        try {
            winExternal.WizDocument.SetHtml2(html, resources);
        } catch (e) {
            console.error(e);
        }
    }

    function checkDocLock(callback) {
        if (isPersonalDocument()) {
            WizReader.todo[callback](false, false);
            return;
        }
        try {
            winExternal.ExecuteCommand("OnClickingChecklist",
                "WizReader.todo." + callback + "({cancel}, {needCallAgain});", "readingnote");
        } catch (e) {
            console.error(e);
        }

    }
}

function routeForWeb() {

    this.getUserAlias = getUserAlias;
    this.getUserGuid = getUserGuid;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.setDocumentType = setDocumentType;
    this.hasPermission = hasPermission;
    //for Reader
    this.getOriginalDoc = getOriginalDoc;
    this.saveDoc = saveDoc;
    this.checkDocLock = checkDocLock;

    function getUserAlias() {
        return 'zzz';
    }

    function getUserGuid() {
        return '63272832-e387-4d31-85c6-7549555f2231';
    }

    function getUserAvatarFileName() {
        return '/wizas/a/users/avatar/63272832-e387-4d31-85c6-7549555f2231?default=true';
    }

    function isPersonalDocument() {
        return false;
        // try {
        // 	return this.wizEditor.WizDocument.IsPersonalDocument();
        // }
        // catch (e) {
        // 	return false;
        // }
    }

    function setDocumentType(type) {
        /*
         var oldType = this.wizDoc.Type;
         if (oldType) {
         if (-1 == oldType.indexOf(type)) {
         this.wizDoc.Type = oldType + ';' + type;
         }
         }
         else {
         this.wizDoc.Type = type;
         }*/
        // this.wizEditor.WizDocument.Type = type;
    }

    function hasPermission() {
        return true;
    }

    function getOriginalDoc() {
        return ENV.doc.body.outerHTML;
    }

    function saveDoc(html, resources) {
        console.log('saveDoc');
    }

    function checkDocLock(callback) {
        setTimeout(function () {
            WizReader.todo.onCheckDocLock(false, false);
        }, 500);
        // if (isPersonalDocument()) {
        //     WizReader.todo.onCheckDocLock(false, false);
        //     return;
        // }
        // this.WizPcEditor.ExecuteCommand("OnClickingChecklist",
        //     "WizReader.todo." + callback + "({cancel}, {needCallAgain});", "readingnote");
    }

}

function routeForMac() {

    qtEditor = ENV.win.WizQtEditor;

    this.getUserAlias = getUserAlias;
    this.getUserGuid = getUserGuid;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.setDocumentType = setDocumentType;
    this.hasPermission = hasPermission;
    //for Reader
    this.getOriginalDoc = getOriginalDoc;
    this.saveDoc = saveDoc;
    this.checkDocLock = checkDocLock;

    function getUserAlias() {
        return qtEditor.userAlias;
    }

    function getUserGuid() {
        return qtEditor.userGuid;
    }

    function getUserAvatarFileName(size) {
        return qtEditor.userAvatarFilePath;
    }

    function isPersonalDocument() {
        return qtEditor.isPersonalDocument;
    }

    function setDocumentType(type) {
        qtEditor.changeCurrentDocumentType(type);
    }

    function hasPermission() {
        return !ENV.readonly || qtEditor.hasEditPermissionOnCurrentNote;
    }

    function getOriginalDoc() {
        return qtEditor.currentNoteHtml;
    }

    function saveDoc(html, resources) {

    }

    function checkDocLock(callback) {
        qtEditor.clickingTodoCallBack.connect(WizReader.todo[callback]);
        return qtEditor.checkListClickable();
    }
}

function routeForAndroid() {

    androidWizNote = ENV.win.WizNote;

    this.getUserAlias = getUserAlias;
    this.getUserGuid = getUserGuid;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.setDocumentType = setDocumentType;
    this.hasPermission = hasPermission;
    //for Reader
    this.getOriginalDoc = getOriginalDoc;
    this.saveDoc = saveDoc;
    this.checkDocLock = checkDocLock;
    this.beforeCloseDoc = beforeCloseDoc;

    function getUserAlias() {
        return androidWizNote.getUserAlias();
    }

    function getUserGuid() {
        return androidWizNote.getUserGuid();
    }

    function getUserAvatarFileName(size) {
        return androidWizNote.getUserAvatarFileName(size);
    }

    function isPersonalDocument() {
        return androidWizNote.isPersonalDocument();
    }

    function setDocumentType(type) {
        androidWizNote.setDocumentType(type);
    }

    function hasPermission() {
        return !ENV.readonly || androidWizNote.hasPermission();
    }

    function getOriginalDoc() {
        return androidWizNote.getDocHtml();
    }

    function saveDoc(html, resources) {
        androidWizNote.setDocHtml(html, resources);
    }

    function checkDocLock(callback) {
        androidWizNote.checkDocLock();
    }

    function beforeCloseDoc() {
        androidWizNote.onWizTodoReadCheckedClose();
    }
}

function routeForIOS() {

    this.getUserAlias = getUserAlias;
    this.getUserGuid = getUserGuid;
    this.getUserAvatarFileName = getUserAvatarFileName;
    this.isPersonalDocument = isPersonalDocument;
    this.setDocumentType = setDocumentType;
    this.setTodoInfo = setTodoInfo;
    this.hasPermission = hasPermission;

    //for Reader
    this.getOriginalDoc = getOriginalDoc;
    this.checkDocLock = checkDocLock;

    this.userAlias = '';
    this.userGuid = '';
    this.avatarFileName = '';
    this.personalDocument = false;
    this._hasPermission = false;
    this.originalHtml = "";

    function setTodoInfo(options) {
        this.userAlias = options.alias;
        this.userGuid = options.userGuid;
        this.avatarFileName = options.avatar;
        this.personalDocument = options.isPersonal === 'true';
        this._hasPermission = options.hasPermission === 'true';
        this.originalHtml = base64.decode(options.docHtml);
    }

    function getUserAlias() {
        return this.userAlias;
    }

    function getUserGuid() {
        return this.userGuid;
    }

    function getUserAvatarFileName(size) {
        return this.avatarFileName;
    }

    function isPersonalDocument() {
        return this.personalDocument;
    }

    function setDocHtml(html, resources) {
        ENV.win.location.href = "wiztodolist://setDocHtml/" + "?html=" + html + "&resource=" + resources;
    }

    function setDocumentType(type) {
        ENV.win.location.href = "wiztodolist://setDocumentType/" + "?type=" + type;
    }

    function hasPermission() {
        return !ENV.readonly || this._hasPermission;
    }

    function getOriginalDoc() {
        return this.originalHtml;
    }

    function checkDocLock(callback) {
        ENV.win.location.href = "wiztodolist://tryLockDocument/" + "?callback=" + callback;
    }
}

var todoClientRoute = {
    getRoute: function () {
        var route = null;
        if (ENV.client.type.isWin) {
            route = new routeForWindows();
        } else if (ENV.client.type.isMac) {
            route = new routeForMac();
        } else if (ENV.client.type.isIOS) {
            route = new routeForIOS();
        } else if (ENV.client.type.isAndroid) {
            route = new routeForAndroid();
        } else {
            route = new routeForWeb();
        }

        return route;
    },
    setQtEditor: function() {
        qtEditor = ENV.win.WizQtEditor;
    }
};

module.exports = todoClientRoute;
},{"../common/base64":17,"../common/env":20}],58:[function(require,module,exports){
/**
 * todoList 相关样式处理
 */
var ENV = require('../common/env'),
  CONST = require('../common/const'),
  wizStyle = require('../common/wizStyle'),
  domUtils = require('../domUtils/domBase');

var TodoStyleMap = {};
var ImgFile = {
  // todoChecked: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyJpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMy1jMDExIDY2LjE0NTY2MSwgMjAxMi8wMi8wNi0xNDo1NjoyNyAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNiAoV2luZG93cykiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6RjY1OTU4MUZCRjk3MTFFM0JENDdFMDk4NDNCMkZDMTQiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6RjY1OTU4MjBCRjk3MTFFM0JENDdFMDk4NDNCMkZDMTQiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDpGNjU5NTgxREJGOTcxMUUzQkQ0N0UwOTg0M0IyRkMxNCIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDpGNjU5NTgxRUJGOTcxMUUzQkQ0N0UwOTg0M0IyRkMxNCIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PqkphX0AAAJZSURBVHjaYvz//z/DQAImhgEGLNu2bYNzCj+7jpgQaAdiUNz/Z4EKqAJxRz/vbhcgzUeBwSBDE4AhuQiPh6cCcQY8CoBYHYhPALEAFXxWiMdyViCeD8TRSGLdIAe0QS3/BcSlQLwEaMg7Kgc5BxCvAmJfJLGlQFwOcoALVKAMaPEkGsQ3LxBvBGJHJLF9QJwESwOwOF9MA8uFgRiUzcyQxM4AcRA0xBlYkCSoHeySQLwbiLWRxO4AsRcQf0ROhLQAikC8B4iVkMReA7EnlCZYDtQDcT+Z5QTIx0fQLP8CTYB3MEpCHJY3QNlyQBwDxN+JtNwEiHdA4x4GQHHtD8QniS0JxZHYoMSyF4hFiLDcAZq6hdEKpiSoONFFcS6aBksgPgbEKngs94Gmdl408SpofiepLvgL9TlyfIGK6uNAbIFFfSQQrwNiTjTxLlDxjiwALOoZia2MPkJT7EckMRFoyAQhiYHK9CXQYpYBrZSrwGJuPym14R2oZX+RxEC+XA3E+aBiFIinYTEDXsqh+b4Vqo9gLkA3LBdqEbKjJ+BQfxq5lMOSHshqD0yHYkIAFGLeaNFGtQZJLr6shKuUo6YDsOUMGPiMq5SjdpMMW84AxXUArlKOFm1C5JxBsJQjqlVMhh5YzuAnVMrRygEMROYKogDj1q1bYQUGI42b4ij2eHl5YaQBIRpaLoQvEX6CsmNp6IAUpIYJhgP2INVeeUAsSEWLBaFmNkP5u7A5oBqIPwAxGxBPhDZO/1MJv4OayQa1oxqbA25A6/l10FKN2uAz1GwLqF0oACDAAGu/mbMal6iXAAAAAElFTkSuQmCC',
  todoChecked: 'data:text/xml;base64,iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyJpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMy1jMDExIDY2LjE0NTY2MSwgMjAxMi8wMi8wNi0xNDo1NjoyNyAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNiAoV2luZG93cykiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6OUYxRkNCNDRFNUYzMTFFN0I3MjU4OUZFNDhFRjQzMjQiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6OUYxRkNCNDVFNUYzMTFFN0I3MjU4OUZFNDhFRjQzMjQiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDo5RjFGQ0I0MkU1RjMxMUU3QjcyNTg5RkU0OEVGNDMyNCIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDo5RjFGQ0I0M0U1RjMxMUU3QjcyNTg5RkU0OEVGNDMyNCIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PoB3J04AAAJhSURBVHja7JjLK0RRHMdnhFiM55RsFJGFrcSOGguDaHZCSRY2Q7PwiAUlj1gMCkvF2FDKZpLXSh7xB1jMf6AmY5RSGt9Tv6vjdO5j7twzzcKvPt07v+ae873n/B5nxp1KpVy5ZHmuHLOcE5QvOqLR6O99KNmZLR2rYFYqiKwBrIU9lz5cSzKYiAXoCF7swGCHdsC47grBGsEDKHPgzUMGYgrAPhjkfBsyQSsk5gtMgQgGjTu8RUXgGPRyviMwIxPko+s0hGwriBcPOAMdnO8GjLItlgnSYuZQgZhKljeghfM9gwDtiG5QM3N6m6rBJWjifDHgBwmjoFZhteAK1HG+V9BF17QL4wII2yykbEVuBTEfFNAx08KoI2aR7mvAEPi0KKYZnFPsaMZipQ882m0dVdw9C75r4LXwXDtlT6VQKEfJb7uXBYUB2sAdqDd4poeyySP456jeZNRcv2llYkJruQetku8PgFNQLPjXWTviHWhNbrvdPkEZkeB8Xlq5AOdjPSlCbcElVOFZybjhTI4fMZr8m/OxVTgBk6zsg13JmL9VWFidZXou7SwTBw/SxPxLbep8/4mvwpJ4cuSAtkdYWdFuYZuVnRiDRqmrV4VVCpJlnmZJvSqs+kwtyzwWK/16VTgbh3w+80yrsK1Dvg3TMq/UrApnS5DLYtZZMrf4yxU/gzSHW/EZ6c88fr/fNIYqFIqpSCeo3+k6rFDQGHdQMxV0xXXnCVDuoJByGnOJPl9YETQP3kAh2KLDfsoh4jRmIc0xb0XQC51zTqnqOm1JGruV5jLOsv+/Y/4FpWk/AgwACweeMaBPu0MAAAAASUVORK5CYII=',
  // todoUnChecked: 'data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAYAAABzenr0AAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyJpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMy1jMDExIDY2LjE0NTY2MSwgMjAxMi8wMi8wNi0xNDo1NjoyNyAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNiAoV2luZG93cykiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6RDcyODY1Q0NCRjk2MTFFMzhGNTBEODZBNTIzNzhDQjQiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6RDcyODY1Q0RCRjk2MTFFMzhGNTBEODZBNTIzNzhDQjQiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDpENzI4NjVDQUJGOTYxMUUzOEY1MEQ4NkE1MjM3OENCNCIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDpENzI4NjVDQkJGOTYxMUUzOEY1MEQ4NkE1MjM3OENCNCIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PpXYwTEAAAD5SURBVHjaYvz//z/DQAImhgEGA+4Alm3bttHdUi8vL4wQUAXitUD8EYj/Uxl/hJqtijUEgFgdiE8AsQCNPMwHxEFA7ATEFkB8Ez0NtEEt/wXE+UAsDMSMVMJCUDN/Qe1owxYCLlB2GRBPorLv30PN5ATiDiB2w5YL+KDsxTRMd7OhNA++bPiOhg54N1oQjTpg1AGjDhh1wKgDRh0w6oAh4QAhGtojhM8Bn6DsWBo6IAVKf8HmgD1QdhcQ5wGxIBUtFoSa2Qzl78LmgGog/gDEbEA8EdqApFav6B3UTDaoHdXYHHAD2mNZB8SfaRD8n6FmW0DtQgEAAQYAS2BO8CD/bL4AAAAASUVORK5CYII='
  todoUnChecked: 'data:text/xml;base64,iVBORw0KGgoAAAANSUhEUgAAACQAAAAkCAYAAADhAJiYAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAyJpVFh0WE1MOmNvbS5hZG9iZS54bXAAAAAAADw/eHBhY2tldCBiZWdpbj0i77u/IiBpZD0iVzVNME1wQ2VoaUh6cmVTek5UY3prYzlkIj8+IDx4OnhtcG1ldGEgeG1sbnM6eD0iYWRvYmU6bnM6bWV0YS8iIHg6eG1wdGs9IkFkb2JlIFhNUCBDb3JlIDUuMy1jMDExIDY2LjE0NTY2MSwgMjAxMi8wMi8wNi0xNDo1NjoyNyAgICAgICAgIj4gPHJkZjpSREYgeG1sbnM6cmRmPSJodHRwOi8vd3d3LnczLm9yZy8xOTk5LzAyLzIyLXJkZi1zeW50YXgtbnMjIj4gPHJkZjpEZXNjcmlwdGlvbiByZGY6YWJvdXQ9IiIgeG1sbnM6eG1wPSJodHRwOi8vbnMuYWRvYmUuY29tL3hhcC8xLjAvIiB4bWxuczp4bXBNTT0iaHR0cDovL25zLmFkb2JlLmNvbS94YXAvMS4wL21tLyIgeG1sbnM6c3RSZWY9Imh0dHA6Ly9ucy5hZG9iZS5jb20veGFwLzEuMC9zVHlwZS9SZXNvdXJjZVJlZiMiIHhtcDpDcmVhdG9yVG9vbD0iQWRvYmUgUGhvdG9zaG9wIENTNiAoV2luZG93cykiIHhtcE1NOkluc3RhbmNlSUQ9InhtcC5paWQ6QjlFNzg2NjRFNUYzMTFFN0IyNkY5NzZGNEEyNTU3NEYiIHhtcE1NOkRvY3VtZW50SUQ9InhtcC5kaWQ6QjlFNzg2NjVFNUYzMTFFN0IyNkY5NzZGNEEyNTU3NEYiPiA8eG1wTU06RGVyaXZlZEZyb20gc3RSZWY6aW5zdGFuY2VJRD0ieG1wLmlpZDpCOUU3ODY2MkU1RjMxMUU3QjI2Rjk3NkY0QTI1NTc0RiIgc3RSZWY6ZG9jdW1lbnRJRD0ieG1wLmRpZDpCOUU3ODY2M0U1RjMxMUU3QjI2Rjk3NkY0QTI1NTc0RiIvPiA8L3JkZjpEZXNjcmlwdGlvbj4gPC9yZGY6UkRGPiA8L3g6eG1wbWV0YT4gPD94cGFja2V0IGVuZD0iciI/PohRJcgAAAEHSURBVHja7JgxCsJAEEWNSEAQ0XgG8QrWIhYp7e2stRMhpRDETltLe8sU4gE8guIdokQrm/UHvoUWZsVdEJmBx5AUMy87m5DEUUrlfinyuR8LEcqKwuuJKIrSVAdT0AZlwz0TsAVjcPR9/70QogF2oGJpEdIL7IIWaIJD1shCytzAENSAYwiPNW/sEWaOjGNKYwQWhlfnxJpFbomOzqZ+7JmVxb27ZC59cpfFFoVieQ6JkAiJkAiJkAiJkAj9gZBnsa/3iVDC3LMo1Ge+6ghtmWdgAKoGRaqsOeHxRkcoAGfggjlfyJUhYtZ02SPQEdrzi3INLhbGdWHtJns9hSP/h0Toy7gLMABy5T6ChFra5QAAAABJRU5ErkJggg=='
};

var CSS = {
  // PC 客户端使用的 Chrome 内核 49.0.2623.110，如果 span 设置 position：relative 会导致 span 内 img 的 offsetTop 异常
  common: '.wiz-todo-main {padding-left: 12px;line-height:30px;}' +
  'li > .wiz-todo-main {padding-left: 0}' +
  '.wiz-todo-checked {color: #666;}' +
  '.wiz-todo-unchecked {text-decoration: initial;}' +
  '.wiz-todo-checked .wiz-todo-checkbox {background-image:url(' + ImgFile.todoChecked + ')}' +
  '.wiz-todo-unchecked .wiz-todo-checkbox {background-image:url(' + ImgFile.todoUnChecked + ')}' +
  '.wiz-todo-checkbox {border-radius:0;position:relative;top:-1px;vertical-align:middle;border:0;background-color:transparent;outline:none;width:18px !important; height:18px !important; cursor:default; padding:0px 10px 0px 5px;-webkit-user-select: none;background-size:18px;background-repeat:no-repeat;background-position:5px;box-sizing:initial;}' +
  '.wiz-todo-avatar {border:0;background-color:transparent;outline:none;width:20px !important; height: 20px !important; vertical-align: -20%; padding:0; margin:0 10px 0 0; border-radius:100%;background-size:20px;background-repeat:no-repeat;}' +
  '.wiz-todo-completed-info {padding-left: 20px;}' +
  //单独出来主要为了兼容旧的 todoList
  'input.wiz-todo-avatar {position:relative;top:-4px;}' +
  '.wiz-todo-account, .wiz-todo-dt { color: #666; }'
};

function getGuidFromStyleId(styleId) {
  var guidReg = new RegExp('^' + CONST.ID.TODO_AVATAR_STYLE + '(.*)$', 'i');
  return styleId.replace(guidReg, '$1');
}

var todoStyle = {
  insertTodoStyle: function (isForced) {
    todoStyle.removeTodoOldStyle();
    var s = ENV.doc.getElementById(CONST.ID.TODO_STYLE);
    if (isForced || !s) {
      wizStyle.replaceStyleById(CONST.ID.TODO_STYLE, CSS.common, false);
    }
    todoStyle.removeUnUsedTodoStyle();

    var tmpTagId = 'wiz_todo_checkbox_img_preview';
    if (ENV.client.type.isIOS && !ENV.doc.body.querySelector('#' + tmpTagId)) {
      // todo 等此 bug 消失后，可删除此部分代码
      // IOS 新内核 bug [5.0 (iPhone; CPU iPhone OS 11_2 like Mac OS X) AppleWebKit/604.4.7 (KHTML, like Gecko) Mobile/15C114]
      // 如果只有一种 checkbox ，会导致改变状态后，看不到图片，预先缓存两种 checkbox 图片进入 页面，可避免此问题
      var tmpTag = ENV.doc.createElement(CONST.TAG.TMP_TAG);
      var imgChecked = ENV.doc.createElement('img');
      var imgUnChecked = ENV.doc.createElement('img');
      imgChecked.src = ImgFile.todoChecked;
      imgUnChecked.src = ImgFile.todoUnChecked;
      tmpTag.setAttribute('contenteditable', 'false');
      domUtils.css(tmpTag, {
        width: 0,
        height: 0,
        opacity: 0,
        overflow: 'hidden',
        display: 'inline-block'
      });
      tmpTag.id = tmpTagId;
      tmpTag.appendChild(imgChecked);
      tmpTag.appendChild(imgUnChecked);
      ENV.doc.body.appendChild(tmpTag);
    }
  },
  removeTodoOldStyle: function () {
    wizStyle.removeStyleById(CONST.ID.TODO_STYLE_OLD);
  },
  removeTodoStyle: function () {
    todoStyle.removeTodoOldStyle();
    var style = ENV.doc.getElementById(CONST.ID.TODO_STYLE);
    domUtils.remove(style);

    var styleList = ENV.doc.querySelectorAll('style'), guid, i;
    for (i = styleList.length - 1; i >= 0; i--) {
      style = styleList[i];
      if (style.id && style.id.indexOf(CONST.ID.TODO_AVATAR_STYLE) === 0) {
        guid = getGuidFromStyleId(style.id);
        TodoStyleMap[guid] = style.innerHTML;
        domUtils.remove(style);
      }
    }
  },
  removeUnUsedTodoStyle: function (guid) {
    var styleList = ENV.doc.querySelectorAll('style'),
      style, sId, sClass, userAvatar, i;

    if (guid) {
      sId = CONST.ID.TODO_AVATAR_STYLE + guid;
      sClass = CONST.CLASS.TODO_USER_AVATAR + guid;
      userAvatar = ENV.doc.querySelector('.' + sClass);
      style = ENV.doc.getElementById(sId);
      if (style && !userAvatar) {
        TodoStyleMap[guid] = style.innerHTML;
        domUtils.remove(style);
      }
      return;
    }

    for (i = styleList.length - 1; i >= 0; i--) {
      style = styleList[i];
      sId = style.id;
      if (sId && sId.indexOf(CONST.ID.TODO_AVATAR_STYLE) === 0) {
        guid = getGuidFromStyleId(sId);
        if (!ENV.doc.querySelector('.' + CONST.CLASS.TODO_USER_AVATAR + guid)) {
          TodoStyleMap[guid] = style.innerHTML;
          domUtils.remove(style);
        }
      }
    }
  },
  /**
   * 专门用于 redo / undo 操作时，恢复 avatar 样式处理
   */
  restoreUserAvatarStyle: function () {
    var guid, styleId;
    for (guid in TodoStyleMap) {
      if (TodoStyleMap.hasOwnProperty(guid)) {
        styleId = CONST.ID.TODO_AVATAR_STYLE + guid;
        if (!ENV.doc.querySelector('#' + styleId) &&
          ENV.doc.querySelector('.' + CONST.CLASS.TODO_USER_AVATAR + guid)) {
          wizStyle.replaceStyleById(styleId, TodoStyleMap[guid], false);
        }
      }
    }
  }
};

module.exports = todoStyle;

},{"../common/const":18,"../common/env":20,"../common/wizStyle":25,"../domUtils/domBase":28}],59:[function(require,module,exports){
/**
 * todolist 基本工具包
 */
var ENV = require('../common/env'),
  CONST = require('../common/const'),
  Lang = require('../common/lang'),
  LANG = Lang.getLang(),
  base64 = require('../common/base64'),
  wizStyle = require('../common/wizStyle'),
  utils = require('../common/utils'),
  domUtils = require('../domUtils/domExtend'),
  rangeUtils = require('../rangeUtils/rangeExtend'),
  historyUtils = require('../common/historyUtils'),
  todoStyle = require('./todoStyle');

// 1像素 的透明图片
var checkboxImg = 'data:image/gif;base64,R0lGODlhAQABAIAAAP///wAAACH5BAEAAAAALAAAAAABAAEAAAICRAEAOw==';

var todoUtils = {
  addUserInfo: function (main, isChecked, todoId, todoRoute) {
    if (!main) {
      return;
    }

    var userGuid, userName, avatarUrl, dt,
      userHtml, span, child, next, i;

    userGuid = todoUtils.deleteUserInfo(main.parentNode);
    if (!isChecked) {
      todoStyle.removeUnUsedTodoStyle(userGuid);
      return;
    }

    userGuid = todoRoute.getUserGuid();
    userName = todoRoute.getUserAlias();
    avatarUrl = todoRoute.getUserAvatarFileName(CONST.CSS.TODO_LIST.IMG_WIDTH);
    dt = todoUtils.getTime();
    todoUtils.setUserAvatarStyle(userGuid, avatarUrl);
    userHtml = todoUtils.getUserInfoHtml(userGuid, userName, dt);

    span = ENV.doc.createElement('span');
    domUtils.addClass(span, CONST.CLASS.TODO_USER_INFO);
    span.innerHTML = userHtml;
    span.setAttribute(CONST.ATTR.TODO_ID, todoId);

    for (i = main.childNodes.length - 1; i >= 0; i--) {
      child = main.childNodes[i];
      if (domUtils.isTag(child, 'br')) {
        main.removeChild(child);
      }
    }
    next = main.nextElementSibling;
    while (next) {
      if (todoUtils.isMain(next)) {
        main.parentElement.insertBefore(span, next);
        break;
      }
      if (domUtils.isTag(next, 'br')) {
        main.parentElement.insertBefore(span, next);
        break;
      }
      next = next.nextElementSibling;
    }
    if (!next) {
      main.parentElement.appendChild(span);
    }
    if (!span.hasChildNodes()) {
      span.appendChild(ENV.doc.createElement('br'));
    }
    rangeUtils.setRange(span, span.childNodes.length);
  },
  /**
   * 判断 dom 是否可以当作 main 容器
   * @param dom
   * @returns {boolean}
   */
  canBeContainer: function (dom) {
    return !domUtils.isTag(dom, ['body', 'blockquote', 'td', 'th']) && domUtils.isBlock(dom);
  },
  canCreateTodo: function () {
    var range = rangeUtils.getRange();
    if (!range) {
      return false;
    }

    if (domUtils.getParentByClass(range.startContainer, CONST.CLASS.CODE_CONTAINER, true)) {
      // CodeMirror 内
      return false;
    }

    var rangeList = rangeUtils.getRangeDomList({
      noSplit: true
    });

    // 判断当前光标范围内是否只有一个 块结构，否则不做任何操作
    var start = domUtils.getBlockParent(rangeList.startDom, true) || ENV.doc.body,
      end = null;
    if (!range.collapsed && rangeList.endDom !== rangeList.startDom) {
      end = domUtils.getBlockParent(rangeList.endDom, true) || ENV.doc.body;
      if (end !== start) {
        return false;
      }
    } else if (range.collapsed && start === ENV.doc.body) {
      start = ENV.doc.body.childNodes[0];
    }

    return {
      rangeList: rangeList,
      start: start,
      end: end
    };
  },
  cancelTodo: function (container, noSetRange) {
    if (!container) {
      return;
    }
    var range = rangeUtils.getRange(), start, startOffset,
      main, todoFirst, userGuid;
    start = range ? range.startContainer : null;
    startOffset = range ? range.startOffset : 0;
    main = todoUtils.getMainInDom(container);
    userGuid = todoUtils.deleteUserInfo(container);
    todoStyle.removeUnUsedTodoStyle(userGuid);
    todoFirst = todoUtils.deleteMain(main);
    todoFirst = todoFirst ? todoFirst.start : null;
    domUtils.removeClass(container, CONST.CLASS.TODO_LAYER);
    if (!todoFirst) {
      todoFirst = ENV.doc.createElement('br');
      container.appendChild(todoFirst);
    }
    if (!noSetRange) {
      if (!start || !start.parentNode) {
        start = todoFirst;
        startOffset = 0;
      }
      rangeUtils.setRange(start, startOffset);
    }
    //修正 todoList style
    todoUtils.checkTodoStyle(false);
  },
  check: function (main, isChecked) {
    if (isChecked) {
      domUtils.removeClass(main, CONST.CLASS.TODO_UNCHECKED);
      domUtils.addClass(main, CONST.CLASS.TODO_CHECKED);
    } else {
      domUtils.removeClass(main, CONST.CLASS.TODO_CHECKED);
      domUtils.addClass(main, CONST.CLASS.TODO_UNCHECKED);
    }
    var check = todoUtils.getCheckbox(main);
    var state = isChecked ? 'checked' : 'unchecked';
    check.setAttribute(CONST.ATTR.TODO_CHECK, state);
  },
  checkTodo: function (checkbox, todoRoute) {
    var result = {
      checkbox: null,
      checked: false
    };

    historyUtils.saveSnap(false);

    var main = todoUtils.getMainFromChild(checkbox),
      isChecked;
    if (!main || main.children[0] != checkbox) {
      todoUtils.fixCheckbox(checkbox, false);
    }

    isChecked = checkbox.getAttribute(CONST.ATTR.TODO_CHECK) !== 'checked';
    todoUtils.check(main, isChecked);
    result.checkbox = checkbox;
    result.checked = isChecked;

    if (!todoRoute.isPersonalDocument()) {
      todoUtils.addUserInfo(main, isChecked, checkbox.id, todoRoute);
    }

    return result;
  },
  checkTodoStyle: function (isForced) {
    var todoObj = ENV.doc.querySelector('.' + CONST.CLASS.TODO_CHECKBOX);
    if (todoObj) {
      todoStyle.insertTodoStyle(isForced);
    } else {
      todoStyle.removeTodoStyle();
    }
  },
  clearBlock: function (dom) {
    if (!dom || (dom.nodeType !== 1 && dom.nodeType !== 3 && dom.nodeType !== 11)) {
      return false;
    }
    var child, i;
    for (i = 0; i < dom.childNodes.length; i++) {
      child = dom.childNodes[i];
      if (todoUtils.clearBlock(child) && child != dom.childNodes[i]) {
        i--;
      }
    }

    var isFragment = dom.nodeType == 11,
      // isMain = isFragment ? false : todoUtils.isMain(dom),
      isTodoTag = isFragment ? false : todoUtils.isTodoTag(dom),
      isBlock = isFragment ? false : domUtils.isBlock(dom);
    if (isBlock && domUtils.isEmptyDom(dom)) {
      //为保证 样式 正常传递， 不能随便 removeChild
      if (dom.children.length > 0) {
        domUtils.peelDom(dom);
      } else {
        domUtils.remove(dom);
        return true;
      }
    } else if (isBlock || isTodoTag) {
      domUtils.peelDom(dom);
    }
    return false;
  },
  /**
   * 清理 TodoList 的 class
   * @param dom
   */
  clearTodoClass: function (dom) {
    if (!dom) {
      return;
    }
    domUtils.removeClass(dom,
      [CONST.CLASS.TODO_ACCOUNT, CONST.CLASS.TODO_AVATAR,
        CONST.CLASS.TODO_DATE, CONST.CLASS.TODO_LAYER,
        CONST.CLASS.TODO_MAIN, CONST.CLASS.TODO_CHECKED,
        CONST.CLASS.TODO_UNCHECKED, CONST.CLASS.TODO_USER_INFO]);
  },
  /**
   * 复制 todo
   * @param container
   * @param todoRoute
   * @returns {*}
   */
  cloneTodo: function (container) {
    if (!container) {
      return null;
    }
    var _container, _main, _last,
      main, last;
    _container = domUtils.clone(container, true);
    main = todoUtils.getMainInDom(container);
    if (!main) {
      return null;
    }
    _main = domUtils.clone(main, true);
    domUtils.removeClass(_main, CONST.CLASS.TODO_CHECKED);
    domUtils.addClass(_main, CONST.CLASS.TODO_UNCHECKED);

    var tmp = main, _tmp = _main;
    while (tmp && tmp.childNodes && tmp.childNodes.length) {
      last = tmp.childNodes[tmp.childNodes.length - 1];

      // 复制 todolist 时， 不复制 块级元素、自闭和标签
      while (!!last &&
      ((last.nodeType === 3 && last.nodeValue.replace(CONST.FILL_CHAR_REG, '').length === 0) ||
        domUtils.isBlock(last) ||
        domUtils.isSelfClosingTag(last))) {
        last = last.previousSibling;
      }

      if (!last || todoUtils.isCheckbox(last)) {
        _last = ENV.doc.createTextNode('');
        last = null;
      } else {
        _last = domUtils.clone(last, true);
      }
      _tmp.appendChild(_last);
      _tmp = _last;
      tmp = last;
    }
    _container.appendChild(_main);
    //使用 setTodo 方式设置 todo

    rangeUtils.setRange(_main, 1);
    return _container;
  },
  deleteMain: function (main) {
    return domUtils.peelDom(main, function (dom) {
      return (!todoUtils.isMain(dom) && !todoUtils.isCheckbox(dom))
    });
  },
  deleteUserInfo: function (container) {
    var userGuid = '';
    var main = todoUtils.getMainInDom(container);

    var userAvatar = container.querySelector('.' + CONST.CLASS.TODO_AVATAR),
      userClass = userAvatar ? userAvatar.className : '',
      guidReg = new RegExp('^(.*' + CONST.CLASS.TODO_USER_AVATAR + ')([^ ]*)(.*)$', 'i');

    if (userClass.indexOf(CONST.CLASS.TODO_USER_AVATAR) > -1) {
      userGuid = userClass.replace(guidReg, '$2');
    }

    var nextSib = main ? main.nextElementSibling : container.firstChild,
      tmpNode;
    while (nextSib) {
      if (todoUtils.isMain(nextSib)) {
        break;
      }
      if (todoUtils.isUserInfo(nextSib)) {
        tmpNode = nextSib;
        nextSib = nextSib.nextElementSibling;
        container.removeChild(tmpNode);
        continue;
      }
      nextSib = nextSib.nextElementSibling;
    }
    return userGuid;
  },
  /**
   * 修正 dom 异常的 checkbox & 由于 execCommand 生成列表时造成的 dom 异常
   * @param checkItem
   * @param isForOld
   */
  fixCheckbox: function (checkItem, isForOld) {
    if (!checkItem) {
      return;
    }
    // 修正缺失 的 id
    if (!checkItem.id) {
      checkItem.id = todoUtils.getCheckId();
    }
    // 清理多余样式（主要避免 粘贴操作带来的影响）
    domUtils.attr(checkItem, {
      'style': null
    });

    //1、ios 端 对于 input type=text 操作异常，
    //2、如果换成 type=button 必须添加 contenteditable = false ；但这样又导致光标上下移动时行为异常
    //3、必须使用自闭和标签，否则会导致可以在 checkbox 内输入内容
    // 因此必须重新使用 img 作为 checkbox 的容器
    if (domUtils.isTag(checkItem, 'input')) {
      let img = ENV.doc.createElement('img');
      img.id = checkItem.id;
      img.className = checkItem.className;
      img.src = checkboxImg;
      img.setAttribute(CONST.ATTR.TODO_CHECK, checkItem.getAttribute(CONST.ATTR.TODO_CHECK));
      domUtils.before(img, checkItem);
      domUtils.remove(checkItem);
      checkItem = img;
    }

    if (!isForOld && todoUtils.getMainFromChild(checkItem)) {
      return;
    }

    var container = domUtils.getBlockParent(checkItem),
      canBeContainer = todoUtils.canBeContainer(container),
      main = ENV.doc.createElement('span'),
      newContainer, next, tmpNext,
      stopInsert = false, dom;
    main.className = (isForOld ? CONST.CLASS.TODO_LABEL_OLD : CONST.CLASS.TODO_MAIN) + ' ' + CONST.CLASS.TODO_UNCHECKED;

    newContainer = ENV.doc.createElement(canBeContainer ? container.tagName : 'div');
    if (canBeContainer) {
      domUtils.after(newContainer, container);
    } else {
      domUtils.before(newContainer, checkItem);
    }
    newContainer.appendChild(main);

    next = checkItem;
    while (next) {
      tmpNext = next.nextSibling;
      if (domUtils.isBlock(next)) {
        if (canBeContainer) {
          stopInsert = true;
          dom = newContainer.nextSibling;
        } else {
          break;
        }
      }
      if (stopInsert) {
        domUtils.before(next, dom);
      } else if (todoUtils.isUserInfo(next)) {
        domUtils.after(next, main);
      } else {
        main.appendChild(next);
      }
      next = tmpNext;
    }

    domUtils.addClass(newContainer, CONST.CLASS.TODO_LAYER);
    // 主要用于修正 将 todoList 设置为 ul/ol 时产生的 错误
    var layer = todoUtils.getContainerFromChild(newContainer.parentNode);
    if (layer) {
      domUtils.removeClass(layer, CONST.CLASS.TODO_LAYER);
    }

    //避免 产生多余的 列表行
    if (canBeContainer && domUtils.isEmptyDom(container)) {
      domUtils.remove(container);
    }
  },
  /**
   * 清理 main 内多余 dom （主要针对 特殊字符 or <br> 结尾的dom）
   * @param main
   */
  fixMain: function (main) {
    if (!main) {
      return;
    }
    var last = domUtils.getLastDeepChild(main),
      check = todoUtils.getCheckbox(main),
      parent, i;

    // 不存在 checkbox 的 main 删除 main class
    if (!check) {
      domUtils.removeClass(main, CONST.CLASS.TODO_MAIN);
    }

    // 清理多余样式（主要避免 粘贴操作带来的影响）
    domUtils.attr(main, {'style': null});

    //保证 main 的 parent 就是 layer
    parent = todoUtils.getContainerFromChild(main);
    if (parent && parent != main && parent !== main.parentNode) {
      try {
        parent.appendChild(main);
      } catch (e) {
        console.error(e);
      }

    }

    //处理 被嵌套的 main
    var subMainList = main.querySelectorAll('.' + CONST.CLASS.TODO_MAIN);
    for (i = subMainList.length - 1; i >= 0; i--) {
      domUtils.after(subMainList[i], main);
    }
    if (domUtils.isEmptyDom(main) && main.parentNode) {
      domUtils.remove(main);
    }

    if (last == check) {
      return;
    }

    if ((last.nodeType !== 1 && last.nodeType !== 3) ||
      (last.nodeType == 1 && domUtils.isTag(last, 'br')) ||
      (last.nodeType === 3 && last.nodeValue.replace(CONST.FILL_CHAR_REG, '').length === 0)) {
      parent = last.parentNode;
      while (parent != main && parent.childNodes.length == 1) {
        last = parent;
        parent = parent.parentNode;
      }
      domUtils.remove(last);

      //避免 br & 特殊符号等元素同时存在，所以需要逐一过滤
      todoUtils.fixMain(main);
    }
  },
  /**
   * 修正 todoList 被嵌套 & 缺失 id 等 的问题
   * @param targetTodo //可以指定 todoItem 修正
   */
  fixNewTodo: function (targetTodo) {
    var i,
      container,
      todoList, todoItem,
      subTodoList, subTodoItem, subParent,
      checkList, checkItem,
      mainList, mainItem,
      start, last, next;

    container = targetTodo ? targetTodo : ENV.doc;

    // 处理那些被嵌套的 todoList（错误的 Dom 结构）
    todoList = container.querySelectorAll('.' + CONST.CLASS.TODO_LAYER);
    for (i = 0; i < todoList.length; i++) {
      todoItem = todoList[i];

      // 清理多余样式（主要避免 粘贴操作带来的影响）
      domUtils.attr(todoItem, {'style': null});

      //清理里面无内容的 Layer
      fixEmptyLayer(todoItem);

      subTodoList = todoItem.querySelectorAll('.' + CONST.CLASS.TODO_LAYER);
      if (subTodoList.length === 0) {
        continue;
      }

      subTodoItem = subTodoList[0];
      subParent = subTodoItem.parentNode;
      if (todoUtils.isTodoTag(subParent) || todoUtils.isLayer(subParent)) {
        start = subTodoItem;
      } else {
        start = subParent;
      }
      last = todoItem;

      if (last === start) {
        continue;
      }

      while (next = start.nextSibling) {
        domUtils.after(next, last);
        last = next;
      }
      domUtils.after(start, todoItem);

      // 不存在 checkbox 的 Layer 删除 Layer class
      fixEmptyLayer(todoItem);
    }

    //修正 Main
    mainList = container.querySelectorAll('.' + CONST.CLASS.TODO_MAIN);
    for (i = 0; i < mainList.length; i++) {
      mainItem = mainList[i];
      todoUtils.fixMain(mainItem);
    }

    // 修正 checkbox
    checkList = container.querySelectorAll('.' + CONST.CLASS.TODO_CHECKBOX);
    for (i = 0; i < checkList.length; i++) {
      checkItem = checkList[i];
      todoUtils.fixCheckbox(checkItem, false);
    }

    function fixEmptyLayer(_container) {
      if (domUtils.isEmptyDom(_container)) {
        domUtils.remove(_container);
        return;
      }

      // 不存在 checkbox 的 Layer 删除 Layer class
      var checkItem = todoUtils.getCheckbox(_container);
      if (!checkItem) {
        domUtils.removeClass(todoItem, CONST.CLASS.TODO_LAYER);
      }
    }
  },
  getCheckbox: function (main) {
    if (!main) {
      return null;
    }
    return main.querySelector('.' + CONST.CLASS.TODO_CHECKBOX);
  },
  getCheckId: function () {
    return 'wiz_todo_' + Date.now() + '_' + Math.floor((Math.random() * 1000000) + 1);
  },
  getContainerFromChild: function (child) {
    if (!child) {
      return null;
    }
    return domUtils.getParentByFilter(child, function (dom) {
      return todoUtils.isLayer(dom);
    }, true);
  },
  getMainByCaret: function () {
    var range = rangeUtils.getRange();
    if (!range) {
      return null;
    }

    var start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
    return todoUtils.getMainFromChild(start.container);
    //
    // var p = domUtils.getParentByFilter(start, function (dom) {
    //     return domUtils.hasClass(dom, CONST.CLASS.TODO_LAYER);
    // }, true);
    // if (!p || !p.hasChildNodes()) {
    //     return null;
    // }
    // return todoUtils.getMainInDom(p);
  },
  getMainFromChild: function (dom) {
    if (!dom) {
      return null;
    }
    return domUtils.getParentByFilter(dom, function (dom) {
      return domUtils.hasClass(dom, CONST.CLASS.TODO_MAIN);
    }, true);
  },
  getMainHtml: function () {
    var str = '<span class="' + CONST.CLASS.TODO_MAIN + ' ' + CONST.CLASS.TODO_UNCHECKED + '">' +
      '<img src="' + checkboxImg + '" id="%1" class="' + CONST.CLASS.TODO_CHECKBOX + ' ' + CONST.CLASS.IMG_NOT_DRAG + '" ' +
      CONST.ATTR.TODO_CHECK + '="unchecked" />' +
      '</span>';
    str = str.replace('%1', todoUtils.getCheckId());
    return str;
  },
  getMainInDom: function (dom) {
    if (!dom || !dom.hasChildNodes())
      return null;
    if (todoUtils.isMain(dom)) {
      return dom;
    }
    if (todoUtils.isLayer(dom)) {
      return dom.querySelector('.' + CONST.CLASS.TODO_MAIN);
    }
    return null;
  },
  getUserInfoInDom: function (dom) {
    return dom.querySelector('.' + CONST.CLASS.TODO_USER_INFO);
  },
  getUserInfoHtml: function (userGuid, userName, dt) {
    var html = '<span class="' + CONST.CLASS.TODO_ACCOUNT + '">' +
      '<input disabled readonly class="%1" />' +
      '%2, ' +
      '</span>' +
      '<span class="' + CONST.CLASS.TODO_DATE + '">%3.</span>';
    var avatarClass = CONST.CLASS.TODO_USER_AVATAR + base64.encode(userGuid);
    return html.replace('%1', CONST.CLASS.IMG_NOT_DRAG + ' ' + CONST.CLASS.TODO_AVATAR + ' ' + avatarClass)
      .replace('%2', userName)
      .replace('%3', dt);
  },
  getTime: function () {
    var dt = new Date();
    var dateStr, timeStr;
    timeStr = getNum(dt.getHours()) + ':' + getNum(dt.getMinutes());

    if (LANG.version == 'en') {
      dateStr = LANG.Month[dt.getMonth()] + ' ' + dt.getDate() + ', ' + dt.getFullYear() + ' at ' + timeStr;
    } else {
      dateStr = dt.getFullYear() + LANG.Date.Year + (dt.getMonth() + 1) + LANG.Date.Month + dt.getDate() + LANG.Date.Day + ' ' + timeStr;
    }
    return dateStr;

    function getNum(num) {
      return (num < 10 ? '0' : '') + num;
    }
  },
  insertToMain: function (doms, main) {
    if (!doms || !main) {
      return;
    }
    var i, dom, last = null;
    for (i = doms.length - 1; i >= 0; i--) {
      dom = doms[i];
      todoUtils.clearTodoClass(dom);
      main.insertBefore(dom, last);
      last = dom;
    }
  },
  /**
   * 判断 光标是否处于 checkbox 后面
   * @returns {*}
   */
  isCaretAfterCheckbox: function () {
    var range = rangeUtils.getRange();
    if (!range) {
      return false;
    }
    var start, prev, main, str;

    main = todoUtils.getMainByCaret();
    if (!main) {
      return false;
    }
    if (range.collapsed) {
      start = rangeUtils.getRangeDetail(range.startContainer, range.startOffset);
      if (start.container.nodeType === 3 && start.offset > 0) {
        str = start.container.nodeValue.substr(0, start.offset);
        if (!utils.isEmpty(str)) {
          return false;
        }
      }

      if (start.isEnd) {
        prev = start.container;
      } else {
        prev = domUtils.getPreviousNode(start.container, false, main);
      }
      return todoUtils.isCheckbox(prev);
    }
    return false;
  },
  /**
   * 判断 光标是否处于 main 最前面
   * （collapsed = false 时，以 end 为准）
   * @returns {*}
   */
  isCaretBeforeCheckbox: function () {
    var result = {
      enable: false,
      checkbox: null
    };
    var range = rangeUtils.getRange();

    if (!range) {
      return result;
    }
    var caretDom = range.endContainer;
    if (caretDom.nodeType === 1) {
      caretDom = caretDom.childNodes[range.endOffset];
    } else if (caretDom.nodeType === 3 && domUtils.isEmptyDom(caretDom)
      && range.endOffset == caretDom.nodeValue.length && !todoUtils.getContainerFromChild(caretDom)) {
      // 如果 textNode 本身已经在 todoContainer 以内 则不进行处理
      caretDom = domUtils.getNextNode(caretDom, false);
      if (caretDom) {
        caretDom = domUtils.getParentByFilter(caretDom, function (dom) {
          return todoUtils.isLayer(dom);
        }, true);
      }
    }

    if (todoUtils.isLayer(caretDom) || todoUtils.isMain(caretDom)) {
      result.enable = true;
      result.checkbox = todoUtils.getCheckbox(caretDom);
    } else if (todoUtils.isCheckbox(caretDom)) {
      result.enable = true;
      result.checkbox = caretDom;
    }
    if (!result.checkbox) {
      // 不存在 checkbox 的 layer 不当做 todoList
      result.enable = false;
    }
    return result
  },
  /**
   * 判断 dom 是否为 todoList 的 checkbox
   * @param dom
   * @returns {*|boolean}
   */
  isCheckbox: function (dom) {
    return domUtils.hasClass(dom, CONST.CLASS.TODO_CHECKBOX);
  },
  isEmptyContainer: function (container) {
    if (!container) {
      return true;
    }
    var childNodes = container.childNodes,
      i, child;

    for (i = 0; i < childNodes.length; i++) {
      child = childNodes[i];
      if (todoUtils.isMain(child)) {
        if (!todoUtils.isEmptyMain(child)) {
          return false;
        }
      } else if (!domUtils.isEmptyDom(child)) {
        return false;
      }
    }
    return true;
  },
  isEmptyMain: function (main) {
    if (!main) {
      return true;
    }
    var childNodes = main.childNodes,
      i, child;

    for (i = 0; i < childNodes.length; i++) {
      child = childNodes[i];
      if (!todoUtils.isCheckbox(child) && !domUtils.isEmptyDom(child)) {
        return false;
      }
    }
    return true;
  },
  /**
   * 判断 dom 是否为 todoList 的 main
   * @param dom
   * @returns {*|boolean}
   */
  isMain: function (dom) {
    return domUtils.hasClass(dom, CONST.CLASS.TODO_MAIN);
  },
  isLayer: function (dom) {
    return domUtils.hasClass(dom, CONST.CLASS.TODO_LAYER);
  },
  /**
   * 判断 dom 是否为 todoList 内特殊 dom
   * @param dom
   * @returns {boolean}
   */
  isTodoTag: function (dom) {
    if (!dom) {
      return false;
    }
    return todoUtils.isMain(dom) ||
      todoUtils.isUserInfo(dom) ||
      domUtils.hasClass(dom, CONST.CLASS.TODO_ACCOUNT) ||
      domUtils.hasClass(dom, CONST.CLASS.TODO_DATE);
  },
  /**
   * 判断 dom 是否为 用户信息
   * @param dom
   * @returns {*|boolean}
   */
  isUserInfo: function (dom) {
    return domUtils.hasClass(dom, CONST.CLASS.TODO_USER_INFO);
  },
  setTodo: function (container, todoRoute) {
    var rangeList, start, end,
      mainHtml, main, userInfo, tmpDom,
      mainChildren = [], child, i, j;

    if (container) {
      //允许指定 容器直接设置 todoList
      start = container;
    } else {
      var canCreate = todoUtils.canCreateTodo();
      if (canCreate === false) {
        return null;
      }

      rangeList = canCreate.rangeList;
      start = canCreate.start;
      // end = canCreate.end;

      // rangeList = rangeUtils.getRangeDomList({
      //     noSplit: true
      // });

      // 如果 start 为 body，则将 startDom 用 div 嵌套
      start = domUtils.packageByDiv(start);

      // start = domUtils.getBlockParent(rangeList.startDom, true) || ENV.doc.body;
      //
      // if (!range.collapsed && rangeList.endDom != rangeList.startDom) {
      //     end = domUtils.getBlockParent(rangeList.endDom, true) || ENV.doc.body;
      //     if (end != start) {
      //         return null;
      //     }
      // }
      if (start !== ENV.doc.body) {
        // 修正 块元素的父节点内 已经存在其他块元素的情况
        var areaStart = domUtils.getPrevBlock(start);
        var areaEnd = domUtils.getNextBlock(start);
        if (areaEnd && areaEnd !== start) {
          domUtils.splitDomBeforeSub(start, areaEnd);
          if (domUtils.isTag(areaEnd, 'br')) {
            domUtils.remove(areaEnd);
          }
        }
        if (areaStart && areaStart !== areaEnd && areaStart !== start) {
          start = domUtils.splitDomBeforeSub(start, areaStart);
          if (domUtils.isTag(areaStart, 'br')) {
            domUtils.remove(areaStart);
          } else {
            domUtils.before(areaStart, start);
          }
        }
      }
    }

    // 首先清理 start 内的 block
    for (i = 0, j = start.childNodes.length; i < j; i++) {
      child = start.childNodes[i];
      if (domUtils.isBlock(child)) {
        domUtils.splitDomBeforeSub(start, child);
        break;
      }
      mainChildren.push(child);
    }

    //判断当前行是否已经是 todoList 结构，如果是，则取消 todoList 结构
    main = todoUtils.getMainInDom(start);
    userInfo = todoUtils.getUserInfoInDom(start);
    var hasCheckbox = !!todoUtils.getCheckbox(main);
    if (main || userInfo) {
      todoUtils.cancelTodo(start);
    }
    if (hasCheckbox) {
      return null;
    }

    mainHtml = todoUtils.getMainHtml();
    tmpDom = ENV.doc.createElement('div');
    domUtils.addClass(tmpDom, CONST.CLASS.TODO_LAYER);
    if (!todoUtils.canBeContainer(start)) {
      mainHtml = '<div class="' + CONST.CLASS.TODO_LAYER + '">' + mainHtml + '</div>';
    } else {
      domUtils.addClass(start, CONST.CLASS.TODO_LAYER);
    }
    tmpDom.innerHTML = mainHtml;
    main = todoUtils.getMainInDom(tmpDom);
    end = main.lastChild;

    todoUtils.insertToMain(mainChildren, main);

    while (tmpDom.lastChild) {
      start.insertBefore(tmpDom.lastChild, start.firstChild);
    }

    if (domUtils.isSelfClosingTag(end)) {
      rangeUtils.setRange(end.parentNode, domUtils.getIndex(end) + 1);
    } else {
      rangeUtils.setRange(end, domUtils.getEndOffset(end));
    }

    //通知客户端笔记被修改
    todoRoute.setDocumentType(CONST.TYPE.TODO);

    //修正 todoList style
    todoUtils.checkTodoStyle(false);

    return main;
  },
  setUserAvatarStyle: function (userGuid, avatarUrl) {
    var guid = base64.encode(userGuid);
    var sId = CONST.ID.TODO_AVATAR_STYLE + guid;
    var sClass = CONST.CLASS.TODO_USER_AVATAR + guid;
    var style = ENV.doc.getElementById(sId);
    if (style) {
      return;
    } else {
      style = ENV.win.parent.document.getElementById(sId);
      if (style) {
        wizStyle.insertStyle({id: sId}, style.innerHTML);
        return;
      }
    }
    domUtils.convertImageToBase64(avatarUrl, 50, 50, function (baseStr) {
      //有可能同时点击多个 todoList
      var style = ENV.doc.getElementById(sId);
      if (style) {
        return;
      }
      wizStyle.insertStyle({id: sId}, '.' + sClass + '{background-image:url(' + baseStr + ');}');
    });
  },
  oldPatch: {
    fixImg: function (img) {
      if (!img) {
        return;
      }
      var iObj = ENV.doc.createElement('img');
      iObj.className = img.className;
      iObj.src = checkboxImg;
      domUtils.removeClass(iObj, CONST.CLASS.TODO_CHECK_IMG_OLD);
      domUtils.addClass(iObj, CONST.CLASS.TODO_CHECKBOX);
      if (img.id) {
        iObj.id = img.id;
      }
      if (img.getAttribute('state')) {
        iObj.setAttribute(CONST.ATTR.TODO_CHECK, img.getAttribute('state'))
      }
      var parent = img.parentNode;
      parent.insertBefore(iObj, img);
      parent.removeChild(img);
    },
    fixLabel: function (label) {
      if (label) {
        domUtils.removeClass(label, CONST.CLASS.TODO_LABEL_OLD);
        domUtils.addClass(label, CONST.CLASS.TODO_MAIN);
        if (domUtils.hasClass(label, CONST.CLASS.TODO_CHECKED_OLD)) {
          domUtils.removeClass(label, CONST.CLASS.TODO_CHECKED_OLD);
          domUtils.addClass(label, CONST.CLASS.TODO_CHECKED);
        } else if (domUtils.hasClass(label, CONST.CLASS.TODO_UNCHECKED_OLD)) {
          domUtils.removeClass(label, CONST.CLASS.TODO_UNCHECKED_OLD);
          domUtils.addClass(label, CONST.CLASS.TODO_UNCHECKED);
        }
      }

      if (!label || domUtils.isTag(label, 'span')) {
        return;
      }
      var parent = label.parentNode;
      if (!parent) {
        return;
      }
      var span = ENV.doc.createElement('span');
      span.className = label.className;
      while (label.firstChild) {
        span.appendChild(label.firstChild);
      }
      parent.insertBefore(span, label);
      parent.removeChild(label);
    },
    /**
     * 初始化 todoList 主要用于修正旧版本的 todoList 样式
     * 保证每个 todoItem 占一行
     */
    fixOldTodo: function () {
      var i, j, subLabelList, subLabel, container, subContainer,
        checkImgList, checkImg, labelList, label, tailList, tail;

      //修正未被 label 封装的 checkImg
      checkImgList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TODO_CHECK_IMG_OLD);
      for (i = checkImgList.length - 1; i >= 0; i--) {
        checkImg = checkImgList[i];
        label = todoUtils.oldPatch.getLabelFromChild(checkImg);
        if (!label || label.children[0] !== checkImg) {
          todoUtils.fixCheckbox(checkImg, true);
        }
        //将 image 转换为 i
        todoUtils.oldPatch.fixImg(checkImg);
      }

      //处理那些被嵌套的 todoList（错误的 Dom 结构）
      labelList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TODO_LABEL_OLD);
      for (i = 0; i < labelList.length; i++) {
        label = labelList[i];
        container = todoUtils.oldPatch.packageTodo(label);
        domUtils.addClass(container, CONST.CLASS.TODO_LAYER);
        subLabelList = container.querySelectorAll('.' + CONST.CLASS.TODO_LABEL_OLD);
        for (j = subLabelList.length - 1; j > 0; j--) {
          subLabel = subLabelList[j];
          subContainer = todoUtils.oldPatch.packageTodo(subLabel);
          domUtils.after(subContainer, container);
        }
        //修正 用户信息
        todoUtils.oldPatch.fixUserInfo(label);
      }

      //将 label 全部替换为 span
      labelList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TODO_LABEL_OLD);
      for (i = labelList.length - 1; i >= 0; i--) {
        todoUtils.oldPatch.fixLabel(labelList[i]);
      }

      //清理 Tail
      tailList = ENV.doc.querySelectorAll('.' + CONST.CLASS.TODO_TAIL_OLD);
      for (i = tailList.length - 1; i >= 0; i--) {
        tail = tailList[i];
        if (domUtils.isEmptyDom(tail)) {
          domUtils.remove(tail);
        } else {
          domUtils.removeClass(CONST.CLASS.TODO_TAIL_OLD);
        }
      }
    },
    fixUserInfo: function (label) {
      var parent = label.parentNode,
        check = todoUtils.oldPatch.getCheckImg(label),
        id = check ? check.id : '',
        childNodes = parent.childNodes,
        child, i, firstUserInfo = false;
      for (i = 0; i < childNodes.length; i++) {
        child = childNodes[i];
        if (domUtils.hasClass(child, CONST.CLASS.TODO_USER_INFO)) {
          if (!firstUserInfo) {
            firstUserInfo = true;
            child.setAttribute(CONST.ATTR.TODO_ID, id);
          } else {
            parent.removeChild(child);
            i--;
          }
        }
      }
    },
    getCheckImg: function (label) {
      if (!label) {
        return null;
      }
      return label.querySelector('.' + CONST.CLASS.TODO_CHECK_IMG_OLD);
    },
    getLabelFromChild: function (dom) {
      if (!dom) {
        return null;
      }
      return domUtils.getParentByFilter(dom, function (dom) {
        return domUtils.hasClass(dom, CONST.CLASS.TODO_LABEL_OLD);
      }, true);
    },
    isFirstLabel: function (label) {
      if (!label) {
        return false;
      }
      var parent = label.parentNode,
        childNodes = parent.childNodes,
        i, child;

      for (i = 0; i < childNodes.length; i++) {
        child = childNodes[i];
        if (child === label) {
          return true;
        } else if (!domUtils.isEmptyDom(child)) {
          return false;
        }
      }
      return false;
    },
    /**
     * 给 todoItem 打包
     * @param label
     * @returns {*}
     */
    packageTodo: function (label) {
      if (!label) {
        return null;
      }
      var parent = label.parentNode;

      if (parent !== ENV.doc.body && todoUtils.oldPatch.isFirstLabel(label)) {
        //如果 label 是首元素，则直接返回 label 的父元素
        return parent;
      }

      // 如果 label 不是首元素，则 对齐进行打包
      var check = todoUtils.oldPatch.getCheckImg(label),
        id = check ? check.id : '',
        userInfo = id ? parent.querySelector('span[' + CONST.ATTR.TODO_ID + '=' + id + ']') : null,
        next = label.nextSibling, tmpNext;
      var container = ENV.doc.createElement('div');
      container.appendChild(label);
      while (next) {
        tmpNext = next.nextSibling;
        container.appendChild(next);
        next = (next === userInfo) ? null : tmpNext;
      }
      parent.insertBefore(container, tmpNext);
      return container;
    }
  }
};

module.exports = todoUtils;

},{"../common/base64":17,"../common/const":18,"../common/env":20,"../common/historyUtils":21,"../common/lang":22,"../common/utils":24,"../common/wizStyle":25,"../domUtils/domExtend":29,"../rangeUtils/rangeExtend":49,"./todoStyle":58}],60:[function(require,module,exports){
var ENV = require('./common/env'),
    WizEditor = require('./WizEditor'),
    WizReader = require('./WizReader'),
    todoClientRoute = require('./todoUtils/todoRouteForClient');

var editorInit = WizEditor.init;
WizEditor.init = function (options) {
    editorInit(options);
    WizReader.init();

    //捕获 Mac 端初始化结束
    ENV.win.initForWebEngine = function() {
        todoClientRoute.setQtEditor();
    };
    return WizEditor;
};


module.exports = WizEditor;
},{"./WizEditor":5,"./WizReader":6,"./common/env":20,"./todoUtils/todoRouteForClient":57}]},{},[60]);
