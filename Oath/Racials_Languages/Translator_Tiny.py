import re
import random

class Translator_Tiny:
    def __init__(self):
        # Pronouns
        self.pronouns = {
            r'\bI\b|\bme\b|\bmyself\b': 'Littl\'un',
            r'\byou\b|\byour\b|\byourself\b': 'Goodfellow',
            r'\bwe\b|\bus\b|\bour\b|\bourselves\b': 'Folksy',
            r'\bthey\b|\bthem\b|\btheir\b|\bthemselves\b': 'Them\'uns',
            r'\bhe\b|\bhim\b|\bhis\b|\bhimself\b': 'Lad\'o',
            r'\bshe\b|\bher\b|\bhers\b|\bherself\b': 'Lass\'ie',
            r'\bit\b|\bits\b|\bitself\b': 'Bitsy'
        }
        
        # Common nouns
        self.nouns = {
            r'\bperson\b|\bpeople\b': 'folk',
            r'\bfriend\b|\bally\b': 'heartkin',
            r'\benemy\b|\bfoe\b': 'troubler',
            r'\bleader\b|\bboss\b|\bchief\b': 'bigtalker',
            r'\bwarrior\b|\bfighter\b|\bsoldier\b': 'bravefolk',
            r'\bfood\b|\bmeal\b': 'nummies',
            r'\bdrink\b|\bbeverage\b': 'sipsy',
            r'\bweapon\b': 'pointystick',
            r'\bhome\b|\bhouse\b|\bshelter\b': 'snughole',
            r'\bmoney\b|\bgold\b|\bcoin\b': 'shinyclinkies',
            r'\btalk\b|\bspeech\b|\bwords\b': 'chitterchatter'
        }
        
        # Verbs
        self.verbs = {
            r'\bam\b|\bare\b|\bis\b|\bwas\b|\bwere\b': 'bein\'',
            r'\bhave\b|\bhas\b|\bhad\b': 'keepin\'',
            r'\bgo\b|\bgoes\b|\bwent\b|\bgoing\b': 'tottle',
            r'\bsee\b|\bsees\b|\bsaw\b|\bseeing\b': 'peepie',
            r'\bhear\b|\bhears\b|\bheard\b|\bhearing\b': 'earful',
            r'\beat\b|\beats\b|\bate\b|\beating\b': 'munchie',
            r'\bdrink\b|\bdrinks\b|\bdrank\b|\bdrinking\b': 'sipsie',
            r'\bfight\b|\bfights\b|\bfought\b|\bfighting\b': 'tussle',
            r'\bhurt\b|\bhurts\b|\bhurt\b|\bhurting\b': 'ouchie',
            r'\bthink\b|\bthinks\b|\bthought\b|\bthinking\b': 'ponderie',
            r'\blike\b|\blikes\b|\bliked\b|\bliking\b': 'heartswell',
            r'\bhate\b|\bhates\b|\bhated\b|\bhating\b': 'frownface'
        }
        
        # Adjectives
        self.adjectives = {
            r'\bgood\b': 'cosylike',
            r'\bbad\b': 'bothersome',
            r'\bbig\b|\blarge\b': 'biggish',
            r'\bsmall\b|\btiny\b': 'teeny-weeny',
            r'\bstrong\b|\bpowerful\b': 'toughsome',
            r'\bweak\b|\bfeeble\b': 'wobbleish',
            r'\bfast\b|\bquick\b': 'quickity',
            r'\bslow\b': 'gentlepace',
            r'\bsmart\b|\bclever\b|\bintelligent\b': 'cleverhead',
            r'\bstupid\b|\bdumb\b': 'plainfolk',
            r'\bhappy\b|\bglad\b|\bjoyful\b': 'smilesome',
            r'\bsad\b|\bunhappy\b|\bupset\b': 'teardrop'
        }
        
        # Other important words
        self.other_words = {
            r'\byes\b': 'surely-so',
            r'\bno\b': 'nope-nope',
            r'\bvery\b|\breally\b': 'quite-quite',
            r'\bmaybe\b|\bperhaps\b': 'mightbe',
            r'\bhello\b|\bhi\b|\bhey\b': 'merry-day',
            r'\bgoodbye\b|\bbye\b': 'bye-bye-now',
            r'\bplease\b': 'kindlydo',
            r'\bthanks\b|\bthank you\b': 'muchgrateful',
            r'\bsorry\b|\bapologies\b': 'apologie-bits'
        }
        
        # Interjections to add randomly
        self.interjections = [
            'Goodness-gracious!', 
            'Oh-my-my!', 
            'Fiddle-faddle!',
            'Butter-biscuits!',
            'Sweet-as-pie!',
            'Teacups-tumbled!'
        ]
    
    def add_diminutives(self, text):
        # Add diminutive suffixes to nouns
        # This is a simplified approach that adds -ie to some nouns
        words = text.split()
        for i, word in enumerate(words):
            # Skip words that are already diminutives or that are in our dictionaries
            if (word.endswith(('y', 'ie', 'let', 'ling')) or 
                any(re.search(pattern, word, re.IGNORECASE) for pattern in [*self.pronouns, *self.nouns, *self.verbs, *self.adjectives, *self.other_words])):
                continue
                
            # Simple heuristic: words that might be nouns (not ending in common verb endings)
            if (len(word) > 3 and 
                not word.endswith(('ing', 'ed', 'ly', 'est', 'er', 'ful')) and 
                random.random() < 0.3):  # 30% chance
                
                if word.endswith('e'):
                    words[i] = word[:-1] + 'ie'
                else:
                    words[i] = word + 'ie'
        
        return ' '.join(words)
    
    def handle_verb_forms(self, text):
        # Present continuous: add -in'
        text = re.sub(r'\b(am|are|is)\s+(\w+)ing\b', r'bein\' \2in\'', text, flags=re.IGNORECASE)
        
        # Past tense: add -ed-up
        text = re.sub(r'\b(\w+)ed\b', r'\1-ed-up', text, flags=re.IGNORECASE)
        
        # Future tense: use gonna
        text = re.sub(r'\bwill\s+(\w+)\b', r'gonna \1', text, flags=re.IGNORECASE)
        
        return text
    
    def handle_questions(self, text):
        if '?' in text:
            # Remove existing question mark
            text = text.replace('?', '')
            # Add Tinyspeak question ending
            if random.random() < 0.5:
                text += ', righto?'
            else:
                text += ', yes-no?'
        return text
    
    def add_repetition(self, text):
        # Add word repetition for emphasis
        words = text.split()
        for i, word in enumerate(words):
            # Only repeat adjectives and adverbs occasionally
            if (len(word) > 3 and 
                (word.endswith('y') or re.search(r'\b(big|small|good|bad|fast|slow|happy|sad)\b', word, re.IGNORECASE)) and
                random.random() < 0.25):  # 25% chance
                words[i] = f"{word}-{word}"
        
        return ' '.join(words)
    
    def handle_intensifiers(self, text):
        # Replace "very" with Tinyspeak intensifiers
        intensifiers = ['quite-quite', 'super-duper', 'extra-much']
        for intensifier in intensifiers:
            text = re.sub(r'\bvery\s+(\w+)\b', f"{random.choice(intensifiers)} \\1", text, flags=re.IGNORECASE)
        
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
        
        # Apply grammar transformations
        text = self.handle_verb_forms(text)
        text = self.handle_questions(text)
        text = self.handle_intensifiers(text)
        text = self.add_repetition(text)
        text = self.add_diminutives(text)
        
        # Clean up multiple spaces
        text = re.sub(r'\s+', ' ', text).strip()
        
        # Add random interjection with 20% chance
        if random.random() < 0.2:
            text += f" {random.choice(self.interjections)}"
        
        # Capitalize first letter
        text = text[0].upper() + text[1:]
        
        return text

def main():
    translator = Translator_Tiny()
    
    print("=== TRANSLATOR TINY ===")
    print("Type 'exit' or 'quit' to end the program.")
    
    while True:
        user_input = input("\nEnter English text: ")
        
        if user_input.lower() in ['exit', 'quit']:
            print("Bye-bye-now, heartkin!")
            break
        
        tinyspeak_text = translator.translate(user_input)
        print("\nTiny: " + tinyspeak_text)

if __name__ == "__main__":
    main()