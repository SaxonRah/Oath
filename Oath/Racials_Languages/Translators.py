import sys
import argparse
import importlib.util
import os

def load_translator_module(translator_path):
    """Load a translator module from a file path."""
    module_name = os.path.basename(translator_path).replace('.py', '')
    spec = importlib.util.spec_from_file_location(module_name, translator_path)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)
    return module

def get_translator_class(module, translator_type):
    """Get the appropriate translator class based on type."""
    translator_classes = {
        'grukk': 'Translator_Grukk',
        'lord': 'Translator_Lord',
        'tiny': 'Translator_Tiny',
        'wild': 'Translator_Wild'
    }
    
    class_name = translator_classes.get(translator_type.lower())
    if not class_name:
        raise ValueError(f"Unknown translator type: {translator_type}")
    
    return getattr(module, class_name)

def translate_file(input_file, output_file, translator_module, translator_type):
    """Translate the contents of input_file and write to output_file."""
    # Get the translator class and instantiate it
    translator_class = get_translator_class(translator_module, translator_type)
    translator = translator_class()
    
    # Read input file
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            text = f.read()
    except Exception as e:
        print(f"Error reading input file: {e}")
        return False
    
    # Translate the text
    translated_text = translator.translate(text)
    
    # Write to output file
    try:
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(translated_text)
        print(f"Translation complete. Output written to {output_file}")
        return True
    except Exception as e:
        print(f"Error writing output file: {e}")
        return False

def translate_all(input_file, output_dir, translator_paths):
    """Translate the input file through all four translators."""
    # Ensure the output directory exists
    os.makedirs(output_dir, exist_ok=True)
    
    # Read input file
    try:
        with open(input_file, 'r', encoding='utf-8') as f:
            text = f.read()
    except Exception as e:
        print(f"Error reading input file: {e}")
        return False
    
    # Get base filename without path and extension
    base_filename = os.path.basename(input_file)
    base_name, _ = os.path.splitext(base_filename)
    
    # List of translator types
    translator_types = ['grukk', 'lord', 'tiny', 'wild']
    
    # Check if we have all the necessary translator paths
    if len(translator_paths) != 4:
        print(f"Error: Need exactly 4 translator paths, but got {len(translator_paths)}")
        return False
    
    # Translate with each translator
    for i, (translator_type, translator_path) in enumerate(zip(translator_types, translator_paths)):
        try:
            # Load translator module
            translator_module = load_translator_module(translator_path)
            
            # Get translator class and instantiate
            translator_class = get_translator_class(translator_module, translator_type)
            translator = translator_class()
            
            # Translate the text
            translated_text = translator.translate(text)
            
            # Create output filename
            output_filename = f"{base_name}_{translator_type}.txt"
            output_path = os.path.join(output_dir, output_filename)
            
            # Write to output file
            with open(output_path, 'w', encoding='utf-8') as f:
                f.write(translated_text)
            
            print(f"Translation ({translator_type}) complete. Output written to {output_path}")
            
        except Exception as e:
            print(f"Error in {translator_type} translation: {e}")
            continue
    
    return True

def main():
    parser = argparse.ArgumentParser(description='Translate text files using fantasy language translators.')
    subparsers = parser.add_subparsers(dest='command', help='Command to run')
    
    # Parser for single translation
    single_parser = subparsers.add_parser('single', help='Translate using a single translator')
    single_parser.add_argument('input_file', help='Path to the input text file')
    single_parser.add_argument('output_file', help='Path to the output text file')
    single_parser.add_argument('translator_type', choices=['grukk', 'lord', 'tiny', 'wild'], 
                               help='Type of translator to use')
    single_parser.add_argument('translator_path', help='Path to the translator Python file')
    
    # Parser for translating with all translators
    all_parser = subparsers.add_parser('all', help='Translate using all four translators')
    all_parser.add_argument('input_file', help='Path to the input text file')
    all_parser.add_argument('output_dir', help='Directory to save output files')
    all_parser.add_argument('--grukk', required=True, help='Path to the Grukk translator Python file')
    all_parser.add_argument('--lord', required=True, help='Path to the Lordspeak translator Python file')
    all_parser.add_argument('--tiny', required=True, help='Path to the Tinyspeak translator Python file')
    all_parser.add_argument('--wild', required=True, help='Path to the Wildspeak translator Python file')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        return 1
    
    try:
        if args.command == 'single':
            translator_module = load_translator_module(args.translator_path)
            translate_file(args.input_file, args.output_file, translator_module, args.translator_type)
        elif args.command == 'all':
            translator_paths = [args.grukk, args.lord, args.tiny, args.wild]
            translate_all(args.input_file, args.output_dir, translator_paths)
    except Exception as e:
        print(f"Error: {e}")
        return 1
    
    return 0

if __name__ == "__main__":
    sys.exit(main())

# python Translators.py single input.txt output.txt translator_type translator_path.py
# python Translators.py all test_passage.txt translated_stories --grukk Translator_Grukk.py --lord Translator_Lord.py --tiny Translator_Tiny.py --wild Translator_Wild.py
