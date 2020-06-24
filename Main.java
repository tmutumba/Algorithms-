import java.io.*; 

class Main {
 private trieNode root;

 public trie() {
  this.root = new trieNode(' ');
 }

  public static void main (String[] args) { 

   void readMyFile() { 
    DataInputStream dis = null; 
    String record = null; 
    int recCount = 0; 

   try { 
    URL url = new URL("https://mcs.msudenver.edu/~gordon/cs4050/hw/words");

    //String url = args[0]; // file of words names on command line
    StreamTokenizer read = new StreamTokenizer( 
                          new BufferedReader(new InputStreamReader(url.openStream())));
    dis = new DataInputStream(read);  

    while ( (record=dis.readLine()) != null ) { 
     recCount++; 
     System.out.println(recCount + ": " + record); 
    } 
   }catch (IOException e) {
   // catch io errors from FileInputStream or readLine() 
     System.out.println("Uh oh, got an IOException error!" + e.getMessage()); 
    }finally { 
      // if the file opened okay, make sure we close it 
      if (dis != null) {
       try {
        dis.close(); 
       }catch (IOException ioe) {
         }
      } 
     } 
  } 
}