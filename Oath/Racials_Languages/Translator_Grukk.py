import re
import random

class Translator_Grukk:
    def __init__(self):
        # Pronouns
        self.pronouns = {
            r'\bI\b|\bme\b|\bmy\b|\bmyself\b': 'Grukk',
            r'\byou\b|\byour\b|\byourself\b': 'Gakh',
            r'\bwe\b|\bus\b|\bour\b|\bourselves\b': 'Grukkaz',
            r'\bthey\b|\bthem\b|\btheir\b|\bthemselves\b': 'Skumz',
            r'\bhe\b|\bhim\b|\bhis\b|\bhimself\b': 'Grok',
            r'\bshe\b|\bher\b|\bhers\b|\bherself\b': 'Grakka',
            r'\bit\b|\bits\b|\bitself\b': 'Zit'
        }
        
        # Common nouns
        self.nouns = {
            r'\bperson\b|\bpeople\b': 'greenskin',
            r'\bfriend\b|\bally\b': 'warbroth',
            r'\benemy\b|\bfoe\b': 'skulltak',
            r'\bleader\b|\bboss\b|\bchief\b': 'warboss',
            r'\bwarrior\b|\bfighter\b|\bsoldier\b': 'krumpa',
            r'\bfood\b|\bmeal\b': 'grubnosh',
            r'\bweapon\b': 'krumper',
            r'\bhome\b|\bhouse\b|\bshelter\b': 'burrow',
            r'\bmoney\b|\bgold\b|\bcoin\b': 'shinyz',
            r'\btalk\b|\bspeech\b|\bwords\b': 'wordspew'
        }
        
        # Verbs
        self.verbs = {
            r'\bam\b|\bare\b|\bis\b|\bwas\b|\bwere\b': 'iz',
            r'\bhave\b|\bhas\b|\bhad\b': 'got',
            r'\bgo\b|\bgoes\b|\bwent\b|\bgoing\b': 'stompa',
            r'\bsee\b|\bsees\b|\bsaw\b|\bseeing\b': 'eyeball',
            r'\bhear\b|\bhears\b|\bheard\b|\bhearing\b': 'eardrum',
            r'\beat\b|\beats\b|\bate\b|\beating\b': 'chomp',
            r'\bfight\b|\bfights\b|\bfought\b|\bfighting\b': 'krump',
            r'\bkill\b|\bkills\b|\bkilled\b|\bkilling\b': 'skullsplat',
            r'\bthink\b|\bthinks\b|\bthought\b|\bthinking\b': 'headpain',
            r'\blike\b|\blikes\b|\bliked\b|\bliking\b': 'grin-at',
            r'\bhate\b|\bhates\b|\bhated\b|\bhating\b': 'rage-at'
        }
        
        # Adjectives
        self.adjectives = {
            r'\bgood\b': 'mosh',
            r'\bbad\b': 'skrag',
            r'\bbig\b|\blarge\b': 'hugga',
            r'\bsmall\b|\btiny\b': 'runty',
            r'\bstrong\b|\bpowerful\b': 'mighty',
            r'\bweak\b|\bfeeble\b': 'puny',
            r'\bfast\b|\bquick\b': 'speedchop',
            r'\bslow\b': 'slugga',
            r'\bsmart\b|\bclever\b|\bintelligent\b': 'kunnin',
            r'\bstupid\b|\bdumb\b': 'numbskull'
        }
        
        # Other important words
        self.other_words = {
            r'\byes\b': 'zug',
            r'\bno\b': 'nub',
            r'\bvery\b|\breally\b': 'blood',
            r'\bmaybe\b|\bperhaps\b': 'mightbe',
            r'\bhello\b|\bhi\b|\bhey\b': 'WAAAGH!',
            r'\bgoodbye\b|\bbye\b': 'skeddadle',
            r'\bplease\b': 'gib-now',
            r'\bthank you\b|\bthanks\b': 'gud-gib',
            r'\bsorry\b|\bapologies\b': 'grovel'
        }
        
        # Articles to remove
        self.articles = [r'\bthe\b', r'\ba\b', r'\ban\b']
        
        # Interjections to add randomly
        self.interjections = ['WAAAGH!', 'ZOG!', 'KRUMP!', 'GRAH!', 'SKRAG IT!']
    
    def translate(self, text):
        # Convert to lowercase for easier matching
        text = text.lower()
        
        # Remove articles
        for article in self.articles:
            text = re.sub(article, '', text)
        
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
        
        # Handle past tense - look for verbs ending in -ed
        text = re.sub(r'\b(\w+)ed\b', r'\1-did', text)
        
        # Handle future tense - look for "will" constructions
        text = re.sub(r'\bwill\s+(\w+)\b', r'gonna \1', text)
        
        # Handle plurals - add z to words ending in s
        text = re.sub(r'\b(\w+)s\b', r'\1z', text)
        
        # Add possessive marker
        text = re.sub(r'\b(Grukk|Gakh|Grukkaz|Skumz|Grok|Grakka)\s+(\w+)\b', r'\1\'z \2', text)
        
        # Convert questions
        if '?' in text:
            text = text.replace('?', ' gah?')
        
        # Add random interjection with 25% chance
        if random.random() < 0.25:
            text += f" {random.choice(self.interjections)}"
        
        # Clean up multiple spaces
        text = re.sub(r'\s+', ' ', text).strip()
        
        # Capitalize sentence beginnings
        sentences = re.split(r'([.!?] )', text)
        result = ""
        for i in range(0, len(sentences), 2):
            if i < len(sentences):
                part = sentences[i]
                if part:
                    sentences[i] = part[0].upper() + part[1:]
                result += sentences[i]
            if i+1 < len(sentences):
                result += sentences[i+1]
        
        if not result:
            result = text.capitalize()
            
        return result

def main():
    translator = Translator_Grukk()
    
    print("=== TRANSLATOR GRUKK ===")
    print("Type 'exit' or 'quit' to end the program.")
    
    while True:
        user_input = input("\nEnter English text: ")
        
        if user_input.lower() in ['exit', 'quit']:
            print("Skeddadle!")
            break
        
        grukk_text = translator.translate(user_input)
        print("\nGrukk: " + grukk_text)

if __name__ == "__main__":
    main()