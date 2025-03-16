import re
import random

class Translator_Wild:
    def __init__(self):
        # Pronouns
        self.pronouns = {
            r'\bI\b|\bme\b|\bmyself\b': 'This-one',
            r'\byou\b|\byour\b|\byourself\b': 'That-one',
            r'\bwe\b|\bus\b|\bour\b|\bourselves\b': 'Pack',
            r'\bthey\b|\bthem\b|\btheir\b|\bthemselves\b': 'Other-kind',
            r'\bhe\b|\bhim\b|\bhis\b|\bhimself\b': 'Male-one',
            r'\bshe\b|\bher\b|\bhers\b|\bherself\b': 'Female-one',
            r'\bit\b|\bits\b|\bitself\b': 'Thing-being'
        }
        
        # Natural references
        self.natural_refs = {
            r'\bsun\b': 'Sky-fire',
            r'\bmoon\b': 'Night-eye',
            r'\bstars\b|\bstar\b': 'Sky-sparks',
            r'\bsky\b|\bheaven\b': 'Above-world',
            r'\bearth\b|\bground\b|\bsoil\b': 'Under-foot',
            r'\bwater\b|\blake\b|\briver\b': 'Life-flow',
            r'\bfire\b|\bflame\b': 'Bright-hunger',
            r'\bwind\b|\bbreeze\b': 'Sky-breath',
            r'\btree\b|\btrees\b': 'Tall-green',
            r'\brock\b|\bstone\b|\bboulder\b': 'Forever-hard'
        }
        
        # Common nouns
        self.nouns = {
            r'\bperson\b|\bpeople\b|\bhuman\b|\bhumans\b': 'Two-leg',
            r'\bfriend\b|\bally\b': 'Path-walker',
            r'\benemy\b|\bfoe\b|\badversary\b': 'Shadow-bringer',
            r'\bleader\b|\bchief\b|\bguide\b': 'Way-shower',
            r'\bwarrior\b|\bfighter\b|\bsoldier\b': 'Fang-fighter',
            r'\bfood\b|\bmeal\b': 'Life-fill',
            r'\bweapon\b|\bsword\b|\bspear\b': 'Claw-metal',
            r'\bhome\b|\bhouse\b|\bshelter\b': 'Sleep-place',
            r'\bmoney\b|\bgold\b|\bcoin\b': 'Strange-metal',
            r'\btalk\b|\bspeech\b|\bwords\b': 'Wind-send'
        }
        
        # Verbs
        self.verbs = {
            r'\bam\b|\bare\b|\bis\b|\bwas\b|\bwere\b': 'Lives-as',
            r'\bhave\b|\bhas\b|\bhad\b': 'Holds',
            r'\bgo\b|\bgoes\b|\bwent\b|\bgoing\b': 'Moves',
            r'\bsee\b|\bsees\b|\bsaw\b|\bseeing\b': 'Eye-catches',
            r'\bhear\b|\bhears\b|\bheard\b|\bhearing\b': 'Ear-takes',
            r'\beat\b|\beats\b|\bate\b|\beating\b': 'Fills-belly',
            r'\bdrink\b|\bdrinks\b|\bdrank\b|\bdrinking\b': 'Takes-water',
            r'\bfight\b|\bfights\b|\bfought\b|\bfighting\b': 'Shows-fangs',
            r'\bkill\b|\bkills\b|\bkilled\b|\bkilling\b': 'Ends-breath',
            r'\bthink\b|\bthinks\b|\bthought\b|\bthinking\b': 'Mind-sees',
            r'\bfeel\b|\bfeels\b|\bfelt\b|\bfeeling\b': 'Blood-knows',
            r'\blike\b|\blikes\b|\bliked\b|\bliking\b': 'Heart-warms',
            r'\bhate\b|\bhates\b|\bhated\b|\bhating\b': 'Skin-crawls'
        }
        
        # Adjectives
        self.adjectives = {
            r'\bgood\b': 'Sun-bright',
            r'\bbad\b': 'Root-rot',
            r'\bbig\b|\blarge\b': 'Many-spans',
            r'\bsmall\b|\btiny\b': 'Seed-size',
            r'\bstrong\b|\bpowerful\b': 'Storm-might',
            r'\bweak\b|\bfeeble\b': 'Leaf-bend',
            r'\bfast\b|\bquick\b': 'Wind-quick',
            r'\bslow\b': 'Stone-pace',
            r'\bsmart\b|\bclever\b|\bintelligent\b': 'Deep-think',
            r'\bstupid\b|\bdumb\b|\bfoolish\b': 'Cloud-head',
            r'\bold\b|\bancient\b|\baged\b': 'Many-seasons',
            r'\byoung\b|\bnew\b|\bfresh\b': 'New-sprout'
        }
        
        # Other important words
        self.other_words = {
            r'\byes\b': 'So-flows',
            r'\bno\b': 'Against-current',
            r'\bvery\b|\breally\b': 'Deep',
            r'\bmaybe\b|\bperhaps\b': 'Mist-see',
            r'\bhello\b|\bhi\b|\bhey\b': 'See-you',
            r'\bgoodbye\b|\bbye\b': 'Until-next-sun',
            r'\bplease\b': 'Ask-favor',
            r'\bthanks\b|\bthank you\b': 'Good-given',
            r'\bsorry\b|\bapologies\b': 'Wrong-path'
        }
        
        # Directions
        self.directions = {
            r'\bnorth\b': 'Cold-direction',
            r'\bsouth\b': 'Warm-direction',
            r'\beast\b': 'Sun-rise',
            r'\bwest\b': 'Sun-fall',
            r'\bup\b|\babove\b': 'Sky-toward',
            r'\bdown\b|\bbelow\b': 'Earth-toward',
            r'\bleft\b': 'Heart-side',
            r'\bright\b': 'Spear-side'
        }
        
        # Time references
        self.time_refs = {
            r'\bmorning\b|\bdawn\b': 'Sun-rise-time',
            r'\bnoon\b|\bmidday\b': 'High-sun',
            r'\bevening\b|\bdusk\b': 'Sun-fall-time',
            r'\bnight\b|\bmidnight\b': 'Dark-time',
            r'\bday\b|\btoday\b': 'Sun-circle',
            r'\byesterday\b': 'Last-sun',
            r'\btomorrow\b': 'Next-sun',
            r'\bweek\b': 'Seven-suns',
            r'\bmonth\b': 'One-moon-cycle',
            r'\byear\b': 'Season-circle'
        }
        
        # Articles to remove
        self.articles = [r'\bthe\b', r'\ba\b', r'\ban\b']
        
        # Interjections
        self.interjections = [
            'Sky-crack!', 
            'Root-deep!', 
            'Thorn-step!',
            'Wind-take!',
            'Blood-rush!',
            'Stone-still!'
        ]
    
    def handle_tenses(self, text):
        # Past tense: add -before
        text = re.sub(r'\b(\w+)ed\b', r'\1-before', text)
        
        # Future tense: add when-comes
        text = re.sub(r'\bwill\s+(\w+)\b', r'when-comes \1', text)
        
        return text
    
    def handle_questions(self, text):
        if '?' in text:
            # Remove existing question mark and reverse subject-verb if possible
            text = text.replace('?', '')
            # Simple heuristic attempt at inversion
            text = re.sub(r'\b(This-one|That-one|Pack|Other-kind)\s+(\w+-\w+)\b', r'\2, \1', text)
        return text
    
    def handle_negation(self, text):
        # Handle negation
        text = re.sub(r'\bnot\b', 'not-', text)
        return text
    
    def add_time_reference(self, text):
        # Randomly add time or natural reference at beginning
        if random.random() < 0.4 and not any(phrase in text.lower() for phrase in ['sun-rise', 'night-eye', 'dark-time']):
            time_refs = [
                'Sun-bright, ', 
                'Night-eye watching, ',
                'When wind-speaks, ',
                'Under sky-spark light, ',
                'As leaves-fall, '
            ]
            return random.choice(time_refs) + text[0].lower() + text[1:]
        return text
    
    def remove_articles(self, text):
        # Remove articles
        for article in self.articles:
            text = re.sub(article, '', text)
        return text
    
    def create_compound_words(self, text):
        # This is a simplistic approach - in a real implementation, you'd want more sophistication
        words = text.split()
        i = 0
        while i < len(words) - 1:
            # If we have an adjective followed by a noun, consider combining them
            if (len(words[i]) < 8 and len(words[i+1]) < 8 and 
                random.random() < 0.3 and
                '-' not in words[i] and '-' not in words[i+1]):
                words[i:i+2] = [words[i] + '-' + words[i+1]]
            else:
                i += 1
        
        return ' '.join(words)
    
    def translate(self, text):
        # Convert to lowercase for easier matching
        text = text.lower()
        
        # Remove articles
        text = self.remove_articles(text)
        
        # Replace words according to our dictionaries
        for pattern, replacement in self.pronouns.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.natural_refs.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.nouns.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.verbs.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.adjectives.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.other_words.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.directions.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        for pattern, replacement in self.time_refs.items():
            text = re.sub(pattern, replacement, text, flags=re.IGNORECASE)
        
        # Apply grammar transformations
        text = self.handle_tenses(text)
        text = self.handle_questions(text)
        text = self.handle_negation(text)
        text = self.create_compound_words(text)
        
        # Clean up multiple spaces
        text = re.sub(r'\s+', ' ', text).strip()
        
        # Add time reference at beginning sometimes
        text = self.add_time_reference(text)
        
        # Add random interjection with 15% chance
        if random.random() < 0.15:
            text += f" {random.choice(self.interjections)}"
        
        # Capitalize first letter
        text = text[0].upper() + text[1:]
        
        return text

def main():
    translator = Translator_Wild()
    
    print("=== TRANSLATOR WILD ===")
    print("Type 'exit' or 'quit' to end the program.")
    
    while True:
        user_input = input("\nEnter English text: ")
        
        if user_input.lower() in ['exit', 'quit']:
            print("Until-next-sun. May paths cross again.")
            break
        
        wildspeak_text = translator.translate(user_input)
        print("\nWild: " + wildspeak_text)

if __name__ == "__main__":
    main()