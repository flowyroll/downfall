all:
	doxygen
	moxygen doxygen/xml
	sed -i s/Summary/API/g api.md
	sed -i 's/public //g' api.md
	sed -i 's/#### `/### `/g' api.md
	sed -i 's/#### Returns/**Returns**/g' api.md
	sed -i 's/#### Parameters/**Parameters**/g' api.md


clean:
	rm -rf api.md doxygen