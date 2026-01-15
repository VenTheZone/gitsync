---
name: big-pickle
description: Access and use OpenCode's free big-pickle model for specialized data processing and serialization tasks. Optimized for handling complex data structures, binary formats, and pickle operations.
agent: data-processor
---

# Big-Pickle Handler

This skill provides optimized access to OpenCode's free big-pickle model for data processing, serialization, and binary format handling tasks.

## When to use this Skill

Use this Skill when:
- Working with Python pickle files
- Serializing/deserializing complex data structures
- Binary data processing and manipulation
- File format conversions involving pickle
- Data caching and persistence operations
- Large dataset handling and optimization

## Quick start

```bash
# Process pickle file with big-pickle
evolve skill run big-pickle "process this data.pkl --output json"

# Convert between formats
evolve skill run big-pickle "convert data.pkl --format csv --output results.csv"
```

## Instructions

### Data Processing
1. Load and validate pickle files
2. Convert between data formats (pickle ↔ JSON ↔ CSV ↔ Excel)
3. Handle large datasets efficiently
4. Optimize pickle size and compression

### Serialization Operations
1. Serialize complex Python objects
2. Handle different pickle protocols
3. Manage compatibility between Python versions
4. Compress and decompress pickle data

### Error Handling
1. Validate pickle file integrity
2. Handle corrupted pickle files gracefully
3. Provide detailed error messages
4. Suggest recovery options

## Examples

### Basic pickle processing
```python
import pickle
import json

# Load data from pickle
with open('data.pkl', 'rb') as f:
    data = pickle.load(f)

# Convert to JSON
with open('output.json', 'w') as f:
    json.dump(data, f, indent=2)
```

### Large dataset handling
```python
import pickle
import pandas as pd

# Process large pickle in chunks
def process_large_pickle(file_path, chunk_size=1000):
    with open(file_path, 'rb') as f:
        while True:
            try:
                chunk = pickle.load(f)
                yield process_chunk(chunk)
            except EOFError:
                break
```

### Format conversion
```python
import pickle
import pandas as pd

def convert_pickle_to_csv(pickle_file, csv_file):
    data = pickle.load(open(pickle_file, 'rb'))
    if isinstance(data, list):
        df = pd.DataFrame(data)
        df.to_csv(csv_file, index=False)
    else:
        # Handle other data types
        pass
```

## Best practices

- Always validate pickle files before loading
- Use appropriate pickle protocol for your Python version
- Compress large pickle files to save space
- Handle binary data with proper error checking
- Document data structure for future reference

## Requirements

- Python 3.7+
- pickle library (built-in)
- pandas (for CSV conversion)
- numpy (for array operations)

## Error handling

Common issues and solutions:
- **EOFError**: Pickle file corrupted or incomplete
- **UnicodeDecodeError**: Text encoding issues
- **MemoryError**: Dataset too large for available RAM
- **ModuleNotFoundError**: Missing dependencies