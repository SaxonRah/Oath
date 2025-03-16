import re
import random

class Translator_Lord:
    def __init__(self):
        # Pronouns
        self.pronouns = {
            r'\bI\b|\bme\b|\bmyself\b': 'Eth',
            r'\byou\b|\byour\b|\byourself\b': 'Vos',  # Default to superior address
            r'\bwe\b|\bus\b|\bour\b|\bourselves\b': 'Ethran',
            r'\bthey\b|\bthem\b|\btheir\b|\bthemselves\b': 'Vosran',
            r'\bhe\b|\bhim\b|\bhis\b|\bhimself\b': 'Vir',
            r'\bshe\b|\bher\b|\bhers\b|\bherself\b': 'Damé',
            r'\bit\b|\bits\b|\bitself\b': 'Atem'
        }
        
        # Common nouns
        self.nouns = {
            r'\bperson\b|\bpeople\b': 'personage',
            r'\bfriend\b|\bally\b': 'allyar',
            r'\benemy\b|\bopponent\b': 'adversar',
            r'\bleader\b|\bruler\b': 'sovereign',
            r'\bwarrior\b|\bsoldier\b': 'valorant',
            r'\bfood\b|\bmeal\b': 'repast',
            r'\bweapon\b': 'armament',
            r'\bhome\b|\bhouse\b|\bdwelling\b': 'domicile',
            r'\bmoney\b|\bcurrency\b': 'coinage',
            r'\btalk\b|\bdiscussion\b|\bconversation\b': 'discourse'
        }
        
        # Verbs
        self.verbs = {
            r'\bam\b|\bare\b|\bis\b|\bwas\b|\bwere\b': 'exist',
            r'\bhave\b|\bhas\b|\bhad\b': 'possess',
            r'\bgo\b|\bgoes\b|\bwent\b|\bgoing\b': 'proceed',
            r'\bsee\b|\bsees\b|\bsaw\b|\bseeing\b': 'observe',
            r'\bhear\b|\bhears\b|\bheard\b|\bhearing\b': 'attend',
            r'\beat\b|\beats\b|\bate\b|\beating\b': 'partake',
            r'\bfight\b|\bfights\b|\bfought\b|\bfighting\b': 'contest',
            r'\bkill\b|\bkills\b|\bkilled\b|\bkilling\b': 'vanquish',
            r'\bthink\b|\bthinks\b|\bthought\b|\bthinking\b': 'contemplate',
            r'\blike\b|\blikes\b|\bliked\b|\bliking\b': 'favor',
            r'\bhate\b|\bhates\b|\bhated\b|\bhating\b': 'disfavor'
        }
        
        # Adjectives
        self.adjectives = {
            r'\bgood\b': 'beneth',
            r'\bbad\b': 'malath',
            r'\bbig\b|\blarge\b': 'grandiose',
            r'\bsmall\b|\btiny\b': 'diminutive',
            r'\bstrong\b|\bpowerful\b': 'formidable',
            r'\bweak\b|\bfeeble\b': 'infirm',
            r'\bfast\b|\bquick\b': 'expeditious',
            r'\bslow\b': 'deliberate',
            r'\bsmart\b|\bclever\b|\bintelligent\b': 'erudite',
            r'\bstupid\b|\bdumb\b': 'unversed'
        }
        
        # Other important words
        self.other_words = {
            r'\byes\b': 'affirm',
            r'\bno\b': 'declin',
            r'\bvery\b|\breally\b': 'most',
            r'\bmaybe\b|\bperhaps\b': 'perchance',
            r'\bhello\b|\bhi\b|\bhey\b': 'salutations',
            r'\bgoodbye\b|\bbye\b': 'adjournment',
            r'\bplease\b': 'wouldst',
            r'\bthanks\b|\bthank you\b': 'gratitudes',
            r'\bsorry\b|\bapologies\b': 'contrition'
        }
        
        # Phrases to begin statements
        self.statement_beginnings = [
            "Indeed, ", 
            "Verily, ", 
            "Most assuredly, ", 
            "Without question, "
        ]
        
        # Honorifics
        self.honorifics = {
            r'\blord\b|\bnoble\b': 'Hault',
            r'\blady\b|\bnoblewoman\b': 'Hautesse',
            r'\bscholar\b|\bprofessor\b': 'Sapient',
            r'\bwarrior\b|\bsoldier\b': 'Valorant',
            r'\belder\b|\bold one\b': 'Venra',
            r'\bmerchant\b|\btrader\b': 'Mercanti',
            r'\bartisan\b|\bcraftsman\b': 'Artisan',
            r'\bcommoner\b|\bpeasant\b': 'Mundane'
        }
    
    def apply_verb_formality(self, text):
        # Add -eth to verbs in formal contexts
        # This is a simplified approach - would need more sophisticated parsing for all cases
        for verb_pattern, lordspeak_verb in self.verbs.items():
            # Match the verb and replace with formal version
            text = re.sub(f"{verb_pattern}", f"{lordspeak_verb}eth", text, flags=re.IGNORECASE)
        
        return text
    
    def handle_questions(self, text):
        if '?' in text:
            # Remove existing question mark
            text = text.replace('?', '')
            # Add formal question ending
            text += ', pray tell?'
        return text
    
    def add_statement_beginning(self, text):
        # Don't add beginnings to questions or commands
        if '?' in text or text.strip().startswith(('Let', 'Pray')):
            return text
        
        # Randomly decide whether to add a formal beginning
        if random.random() < 0.7:  # 70% chance
            return random.choice(self.statement_beginnings) + text[0].lower() + text[1:]
        return text
    
    def handle_tenses(self, text):
        # Handle past tense (simplified)
        text = re.sub(r'\b(\w+)ed\b', r'\1-ed', text)
        
        # Handle future tense
        text = re.sub(r'\bwill\s+(\w+)\b', r'shall \1', text)
        
        return text
    
    def handle_possessives(self, text):
        # Convert possessives
        for pronoun_pattern, lordspeak_pronoun in self.pronouns.items():
            # Match possessive forms and replace
            possessive_pattern = pronoun_pattern.replace(r'\b', r'\b') + r'\'s\b'
            replacement = f"{lordspeak_pronoun}-iel"
            text = re.sub(possessive_pattern, replacement, text, flags=re.IGNORECASE)
        
        return text
    
    def translate(self, text):
        # Convert to lowercase for easier matching
        text = text.lower()
        
        # Replace words according to our dictionaries
        for pattern, replacement in self.pronouns.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.nouns.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.verbs.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.adjectives.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.other_words.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.honorifics.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        # Apply grammar transformations
        text = self.handle_possessives(text)
        text = self.handle_tenses(text)
        text = self.apply_verb_formality(text)
        text = self.handle_questions(text)
        
        # Clean up multiple spaces
        text = re.sub(r'\s+', ' ', text).strip()
        
        # Add statement beginnings for non-questions
        text = self.add_statement_beginning(text)
        
        # Capitalize first letter
        text = text[0].upper() + text[1:]
        
        return text

def main():
    translator = Translator_Lord()
    
    print("=== TRANSLATOR LORD ===")
    print("Type 'exit' or 'quit' to end the program.")
    
    while True:
        user_input = input("\nEnter English text: ")
        
        if user_input.lower() in ['exit', 'quit']:
            print("Adjournment, good personage!")
            break
        
        lordspeak_text = translator.translate(user_input)
        print("\nLord: " + lordspeak_text)

if __name__ == "__main__":
    main()