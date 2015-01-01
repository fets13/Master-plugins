--[[ 

]]--
function init()
	print ("test::init");
end
function action()
---	print ("test");
	val = getVal("123.value")
	print(val)
	if val == nil then return end
--	val = val + 1
--	setVal("123.value", val)
-- print(val)
end
