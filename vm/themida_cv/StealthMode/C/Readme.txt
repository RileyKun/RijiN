There are three ways to insert a Stealth Area in your application:

1) Stealth Code Area: It will insert and run the protection code and Virtual Machine inside your 
                      own code section (mixed with your code)

2) Stealth Data Area: It will insert and run the protection code and Virtual Machine inside your 
                      global data variables (mixed with your data)

3) Stealth Code-Regs Area: It will insert the Virtual Machine code inside your own code section.
                      The Virtual Machine registers will be inserted in your data section. This is
                      to avoid changing section attributtes for your code and data section.
                      

You should select one or another depending where you prefer the protection code being inserted. The common way is
to insert it inside the code section (Stealth Code Area) 