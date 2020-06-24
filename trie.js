class Trie {
    isWord = false;
    children = {};

    /**
     * Creates a Trie from a list of words.
     * Assumes all elements in `words` are full words.
     * @param {String[]} words list of words from which to create a Trie. 
     * @returns {Node} the TrieNode root of all input `words`.
     */
    constructor(words) {
        var self = this;
        if (words == undefined) return;
        words.forEach(function (word) {
            var curr = self;
            var l = word.length;
            word.split('').forEach(function (c, idx) {
                var sub = curr.children[c] || new Trie();
                if (idx == l - 1) {
                    sub.isWord = true;
                }
                curr.children[c] = sub;
                curr = sub;
            });
        });
    }

    /**
     * Recursive prefix search.
     * @param {String} prefix Prefix of the word to search.
     * @param {Integer} size number of words to return.
     * @returns {String[]} The first `size` words that start with `prefix` in the Trie.
     */
    findWithPrefix(prefix, size) {
        var curr = this;
        var words = [];
        var count = 0;
    
        if (prefix.length > 0 && !Object.keys(this.children).includes(prefix[0])) {
            return [];
        }
    
        prefix.split('').forEach(function (c) {
            curr = curr.children[c] || new Trie();
        });
    
        if (curr.isWord) {
            words.push(prefix);
            count++;
        }
        
        Object.keys(curr.children).forEach(c => {
            curr.children[c].findWithPrefix('', size-count).forEach(suffix => {
                if (count < size)
                {
                    words.push(prefix + c + suffix);
                    count++;
                }
            });
        });
    
        return words.slice(0, size);
    }
}



