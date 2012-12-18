Reuters-21578 is the data set we use to test for index compatibility.

Retuers-21578-index directory contains an index created using java lucene 
version 1.4.3. The following changes were made to the java demo to create 
this index:

* The FileDocument is using f.getName() instead of f.getPath() for the path field.
* The modified field was removed
* The indexwriter must not use compound file: writer.setUseCompoundFile(false)
* The files are sorted using java.util.Arrays.sort(files, String.CASE_INSENSITIVE_ORDER);
* Used a special analyser instead of StandardAnalyzer. This is because the text 
  classification of java differs to that of clucene. See the TestReuters.cpp code
  for implementation. Java version used exactly the same implementation.

The java equivalent analyzer is:

  static class ReutersTokenizer extends org.apache.lucene.analysis.CharTokenizer {
    // Construct a new LetterTokenizer.
    public ReutersTokenizer(java.io.Reader in){
      super(in);
    }
    protected boolean isTokenChar(char c){
      if ( c == ' ' || c == '\t' ||
        c == '-' || c == '.' ||
        c == '\n' || c == '\r' ||
        c == ',' || c == '<' ||
        c == '>' || c<=9){
          return false;
      }else
          return true;
    }
    protected char normalize(char c){
      return c;
    }
  }

  static class ReutersAnalyzer extends org.apache.lucene.analysis.Analyzer {
    public org.apache.lucene.analysis.TokenStream tokenStream(String fieldName, java.io.Reader reader){
      return new ReutersTokenizer(reader);
    }
  };
