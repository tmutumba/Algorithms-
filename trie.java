import java.util.*; 
import java.io.*;
import java.net.*;
import java.util.Scanner;
import java.util.ArrayList;
import java.util.LinkedList;

public class trie {
	private trieNode root;

	public trie() {
		this.root = new trieNode(' ');
	}

	public void isWord(String word) {
		char[] currentChar = word.toCharArray();
		trieNode node = root;
		trieNode tnode = null;
		int index = 0;

		do {
			tnode = node.children[currentChar[index] - 'A'];
			if (tnode != null) {
				node = tnode;
				index++;

				//if the word got over, then it is already in the trie.
				if (index >= word.length()) {
					node.terminal = true;
					node.word = word;
					return;
				}
			}
		}while (tnode != null);

		//temp is the last node and word to add is lengthier
		for (; index < currentChar.length; index++) {
			node.children[currentChar[index] - 'A'] = new trieNode(currentChar[index]);
			node = node.children[currentChar[index] - 'A'];
		}

		node.terminal = true;
		node.word = word;
	}

	public String[] wordByPrefix(String prefix) {
		char[] currentChar = prefix.toCharArray();
		trieNode node = root;
		trieNode tnode = null;
		int index = 0;

		do {
			tnode = node.children[currentChar[index] - 'A'];
			//if you reached the end of the string
			if (tnode == null) {
				return null;
			}

			index++;
			node = tnode;
		}while (index < currentChar.length);

		//node is the node resp of the last char so do traversal for all paths
		List<String> words = new ArrayList<String> ();
		Deque<trieNode> dq = new ArrayDeque<trieNode> ();
		dq.addLast(node);
		while (!dq.isEmpty()) {
			trieNode first = dq.removeFirst ();
			if(first.terminal) {
				words.add(first.word);
			}

			for (trieNode n : first.children) {
				if (n != null) {
					dq.add(n);
				}
			}
		}
		return words.toArray(new String[0]);
	}
}// end trie